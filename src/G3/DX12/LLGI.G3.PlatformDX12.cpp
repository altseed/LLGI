#include "LLGI.G3.PlatformDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

namespace LLGI
{
namespace G3
{

LRESULT LLGI_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void PlatformDX12::Wait()
{
	if (fence == nullptr) return;

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

	DestroyWindow(hwnd);
	UnregisterClassA("DirectX12", GetModuleHandle(NULL));
}

bool PlatformDX12::Initialize(Vec2I windowSize)
{
	// Windows

	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = (WNDPROC)LLGI_WndProc;
	wcex.lpszClassName = "DirectX12";
	wcex.hInstance = GetModuleHandle(NULL);

	RegisterClassExA(&wcex);

	hwnd = CreateWindowA(
		"DirectX12", 
		"DirectX12", 
		WS_OVERLAPPEDWINDOW, 
		100, 
		100, 
		1280, 
		720, 
		NULL, 
		NULL, 
		wcex.hInstance, 
		NULL);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	// DirectX12
	HRESULT hr;

	UINT flagsDXGI = 0;

#if defined(_DEBUG)
	ID3D12Debug *debug_ = nullptr;
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
	hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_1,
		__uuidof(ID3D12Device),
		(void**)&device);

	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// Command queue

	D3D12_COMMAND_QUEUE_DESC descCommandQueue;
	ZeroMemory(&descCommandQueue, sizeof(descCommandQueue));
	descCommandQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	descCommandQueue.Priority = 0;
	descCommandQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	descCommandQueue.NodeMask = 0;
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
	DXGISwapChainDesc.OutputWindow = hwnd;
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

	// Command
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

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
	D3D12_DESCRIPTOR_HEAP_DESC RenderPassHeapDesc;

	// Render target DescriptorHeap
	ZeroMemory(&RenderPassHeapDesc, sizeof(RenderPassHeapDesc));
	RenderPassHeapDesc.NumDescriptors = 2;
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


void PlatformDX12::NewFrame()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		else
		{
			break;
		}
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

	ID3D12CommandList* commandList[] = { commandListStart };
	commandQueue->ExecuteCommandLists(1, commandList);

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

	ID3D12CommandList* commandList[] = { commandListPresent };
	commandQueue->ExecuteCommandLists(1, commandList);

	swapChain->Present(1, 0);
	Wait();
}

Graphics* PlatformDX12::CreateGraphics()
{
	std::function<std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc = [this]() ->std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>
	{
		std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*> ret(handleRTV[frameIndex], RenderPass[frameIndex]);
		
		return ret;
	};

	auto graphics = new GraphicsDX12(
		device,
		getScreenFunc,
		commandQueue);

	return graphics;
}

ID3D12Device* PlatformDX12::GetDevice()
{
	return device;
}

}
}
