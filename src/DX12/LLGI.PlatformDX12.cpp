#include "LLGI.PlatformDX12.h"
#include "../Win/LLGI.WindowWin.h"
#include "LLGI.GraphicsDX12.h"

namespace LLGI
{

void PlatformDX12::Wait()
{
	if (fence == nullptr)
		return;

	HRESULT hr;

	auto fenceValue_ = fenceValue;
	hr = commandQueue->Signal(fence, fenceValue_);
	if (FAILED(hr))
	{
		return;
	}

	fenceValue++;

	if (fence->GetCompletedValue() < fenceValue_)
	{
		hr = fence->SetEventOnCompletion(fenceValue_, fenceEvent);
		if (FAILED(hr))
		{
			return;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

PlatformDX12::PlatformDX12()
{
	for (int32_t i = 0; i < SwapBufferCount; i++)
	{
		RenderPass[i] = nullptr;
	}
}

PlatformDX12::~PlatformDX12()
{
	Wait();

	SafeRelease(descriptorHeapRTV);

	for (int32_t i = 0; i < SwapBufferCount; i++)
	{
		SafeRelease(RenderPass[i]);
		handleRTV[i] = {};
	}

	SafeRelease(commandAllocator);
	SafeRelease(commandListStart);
	SafeRelease(commandListPresent);
	SafeRelease(commandQueue);
	SafeRelease(fence);
	SafeRelease(device);
	SafeRelease(dxgiFactory);
	SafeRelease(swapChain);

	if (fenceEvent != nullptr)
	{
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	window.Terminate();
}

bool PlatformDX12::Initialize(Vec2I windowSize)
{
	// Windows
	if (!window.Initialize("DirectX12", windowSize))
	{
		return false;
	}

	// DirectX12
	HRESULT hr;

	UINT flagsDXGI = 0;

#if defined(_DEBUG)
	ID3D12Debug* debug_ = nullptr;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// make debug enabled
	debug_->EnableDebugLayer();
	debug_->Release();
	debug_ = nullptr;

	flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// factory
	hr = CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// device

	IDXGIAdapter1* adapter = nullptr;

	for (UINT adapterIndex = 0; dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&device)))
		{
			SafeRelease(adapter);
			break;
		}

		SafeRelease(adapter);
	}

	if (device == nullptr)
	{
		goto FAILED_EXIT;
	}

#if defined(_DEBUG)
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(&infoQueue)))
	{
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(severities);
		NewFilter.DenyList.pSeverityList = severities;
		NewFilter.DenyList.NumIDs = _countof(denyIds);
		NewFilter.DenyList.pIDList = denyIds;

		infoQueue->PushStorageFilter(&NewFilter);
		SafeRelease(infoQueue);
	}
#endif

	/*
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&device);

	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	*/

	// Create Command Queue
	D3D12_COMMAND_QUEUE_DESC descCommandQueue;
	ZeroMemory(&descCommandQueue, sizeof(descCommandQueue));
	descCommandQueue.Priority = 0;
	descCommandQueue.NodeMask = 0;
	descCommandQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	descCommandQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = device->CreateCommandQueue(&descCommandQueue, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// Fence
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Swap chain
	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc;
	ZeroMemory(&DXGISwapChainDesc, sizeof(DXGISwapChainDesc));

	DXGISwapChainDesc.BufferDesc.Width = windowSize.X;
	DXGISwapChainDesc.BufferDesc.Height = windowSize.Y;
	DXGISwapChainDesc.OutputWindow = window.GetHandle();
	DXGISwapChainDesc.Windowed = TRUE;
	DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	DXGISwapChainDesc.BufferCount = SwapBufferCount;
	DXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	DXGISwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	DXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGISwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	DXGISwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	DXGISwapChainDesc.SampleDesc.Count = 1;
	DXGISwapChainDesc.SampleDesc.Quality = 0;

	IDXGISwapChain* swapChain_ = nullptr;
	hr = dxgiFactory->CreateSwapChain(commandQueue, &DXGISwapChainDesc, &swapChain_);
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = swapChain_->QueryInterface(&swapChain);
	if (FAILED(hr))
	{
		SafeRelease(swapChain_);
		goto FAILED_EXIT;
	}
	SafeRelease(swapChain_);

	// Create Command Allocator
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// Create Command List
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandListStart));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	commandListStart->Close();

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandListPresent));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	commandListPresent->Close();

	// Render target
	D3D12_DESCRIPTOR_HEAP_DESC RenderPassHeapDesc = {};

	// Render target DescriptorHeap
	RenderPassHeapDesc.NumDescriptors = SwapBufferCount;
	RenderPassHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RenderPassHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RenderPassHeapDesc.NodeMask = 0;
	hr = device->CreateDescriptorHeap(&RenderPassHeapDesc, IID_PPV_ARGS(&descriptorHeapRTV));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	auto descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (int32_t i = 0; i < SwapBufferCount; ++i)
	{

		// get render target from swap chain
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&RenderPass[i]));
		if (FAILED(hr))
		{
			goto FAILED_EXIT;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		handleRTV[i] = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
		handleRTV[i].ptr += descriptorHandleIncrementSize * i;
		device->CreateRenderTargetView(RenderPass[i], nullptr, handleRTV[i]);
	}

	return true;

FAILED_EXIT:;

	SafeRelease(descriptorHeapRTV);

	for (int32_t i = 0; i < SwapBufferCount; i++)
	{
		SafeRelease(RenderPass[i]);
		handleRTV[i] = {};
	}

	SafeRelease(commandAllocator);
	SafeRelease(commandListStart);
	SafeRelease(commandListPresent);
	SafeRelease(commandQueue);
	SafeRelease(fence);
	SafeRelease(device);
	SafeRelease(dxgiFactory);
	SafeRelease(swapChain);

	if (fenceEvent != nullptr)
	{
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	return false;
}

bool PlatformDX12::NewFrame()
{
	if (!window.DoEvent())
	{
		return false;
	}

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	commandListStart->Reset(commandAllocator, nullptr);

	D3D12_RESOURCE_BARRIER barrier;
	ZeroMemory(&barrier, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = RenderPass[frameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandListStart->ResourceBarrier(1, &barrier);
	commandListStart->OMSetRenderTargets(1, &(handleRTV[frameIndex]), FALSE, nullptr);
	commandListStart->Close();

	ID3D12CommandList* commandList[] = {commandListStart};
	commandQueue->ExecuteCommandLists(1, commandList);

	return true;
}

void PlatformDX12::Present()
{
	commandListPresent->Reset(commandAllocator, nullptr);

	D3D12_RESOURCE_BARRIER barrier;
	ZeroMemory(&barrier, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = RenderPass[frameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	commandListPresent->ResourceBarrier(1, &barrier);
	commandListPresent->Close();

	ID3D12CommandList* commandList[] = {commandListPresent};
	commandQueue->ExecuteCommandLists(1, commandList);

	swapChain->Present(1, 0);
	Wait();
}

Graphics* PlatformDX12::CreateGraphics()
{
	std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc =
		[this]() -> std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*> {
		std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*> ret(handleRTV[frameIndex], RenderPass[frameIndex]);

		return ret;
	};

	std::function<void()> waitFunc = [this]() -> void { this->Wait(); };

	auto graphics = new GraphicsDX12(device, getScreenFunc, waitFunc, commandQueue, SwapBufferCount);

	graphics->SetWindowSize(Vec2I(1280, 720));

	return graphics;
}

ID3D12Device* PlatformDX12::GetDevice() { return device; }

} // namespace LLGI
