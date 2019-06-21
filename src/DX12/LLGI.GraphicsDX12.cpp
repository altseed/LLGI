#include "LLGI.GraphicsDX12.h"
#include "LLGI.CommandListDX12.h"
#include "LLGI.ConstantBufferDX12.h"
#include "LLGI.IndexBufferDX12.h"
#include "LLGI.PipelineStateDX12.h"
#include "LLGI.ShaderDX12.h"
#include "LLGI.TextureDX12.h"
#include "LLGI.VertexBufferDX12.h"

namespace LLGI
{

GraphicsDX12::GraphicsDX12(ID3D12Device* device,
						   std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc,
						   std::function<void()> waitFunc,
						   ID3D12CommandQueue* commandQueue,
						   int32_t swapBufferCount)
	: device_(device)
	, getScreenFunc_(getScreenFunc)
	, waitFunc_(waitFunc)
	, commandQueue_(commandQueue)
	, currentScreen(this, false)
	, swapBufferCount_(swapBufferCount)
{
	SafeAddRef(device_);
	SafeAddRef(commandQueue_);

	HRESULT hr;
	// Create Command Allocator
	hr = device->CreateCommandAllocator(commandListType_, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));
}

GraphicsDX12::~GraphicsDX12()
{
	WaitFinish();

	SafeRelease(device_);
	SafeRelease(commandQueue_);
	SafeRelease(commandAllocator_);
}

void GraphicsDX12::NewFrame() { currentSwapBufferIndex = (currentSwapBufferIndex + 1) % swapBufferCount_; }

void GraphicsDX12::Execute(CommandList* commandList)
{
	auto cl = (CommandListDX12*)commandList;
	auto cl_internal = cl->GetCommandList();
	commandQueue_->ExecuteCommandLists(1, (ID3D12CommandList**)(&cl_internal));
}

void GraphicsDX12::WaitFinish()
{
	if (waitFunc_ != nullptr)
	{
		waitFunc_();
	}
}

RenderPass* GraphicsDX12::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	auto currentParam = getScreenFunc_();
	currentScreen.handleRTV = std::get<0>(currentParam);
	currentScreen.RenderPass = std::get<1>(currentParam);
	currentScreen.SetClearColor(clearColor);
	currentScreen.SetIsColorCleared(isColorCleared);
	currentScreen.SetIsDepthCleared(isDepthCleared);
	currentScreen.screenWindowSize = windowSize_;

	return &currentScreen;
}

VertexBuffer* GraphicsDX12::CreateVertexBuffer(int32_t size)
{
	auto obj = new VertexBufferDX12();
	if (!obj->Initialize(this, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

IndexBuffer* GraphicsDX12::CreateIndexBuffer(int32_t stride, int32_t count)
{
	auto obj = new IndexBufferDX12();
	if (!obj->Initialize(this, stride, count))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

ConstantBuffer* GraphicsDX12::CreateConstantBuffer(int32_t size, ConstantBufferType type)
{
	if (type == ConstantBufferType::ShortTime)
	{
		throw "Not inplemented";
	}

	auto obj = new ConstantBufferDX12();
	if (!obj->Initialize(this, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

Shader* GraphicsDX12::CreateShader(DataStructure* data, int32_t count)
{
	auto obj = new ShaderDX12();
	if (!obj->Initialize(data, count))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

CommandList* GraphicsDX12::CreateCommandList()
{
	auto obj = new CommandListDX12();
	if (!obj->Initialize(this))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

PipelineState* GraphicsDX12::CreatePiplineState() { return new PipelineStateDX12(this); }

RenderPass* GraphicsDX12::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	throw "Not inplemented";
}

Texture* GraphicsDX12::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	auto obj = new TextureDX12(this);
	if (!obj->Initialize(size, isRenderPass, isDepthBuffer))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

Texture* GraphicsDX12::CreateTexture(uint64_t id) { throw "Not inplemented"; }

std::shared_ptr<RenderPassPipelineStateDX12> GraphicsDX12::CreateRenderPassPipelineState(bool isPresentMode, bool hasDepth)
{
	RenderPassPipelineStateDX12Key key;
	key.isPresentMode = isPresentMode;
	key.hasDepth = hasDepth;

	// already?
	{
		auto it = renderPassPipelineStates.find(key);

		if (it != renderPassPipelineStates.end())
		{
			auto ret = it->second.lock();

			if (ret != nullptr)
				return ret;
		}
	}

	std::shared_ptr<RenderPassPipelineStateDX12> ret = std::make_shared<RenderPassPipelineStateDX12>(this);

	renderPassPipelineStates[key] = ret;

	return ret;
}

ID3D12Device* GraphicsDX12::GetDevice() { return device_; }

int32_t GraphicsDX12::GetCurrentSwapBufferIndex() const { return currentSwapBufferIndex; }

int32_t GraphicsDX12::GetSwapBufferCount() const { return swapBufferCount_; }

ID3D12Resource* GraphicsDX12::CreateResource(D3D12_HEAP_TYPE heapType,
											 DXGI_FORMAT format,
											 D3D12_RESOURCE_DIMENSION resourceDimention,
											 D3D12_RESOURCE_STATES resourceState,
											 Vec2I size)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	D3D12_RESOURCE_DESC resDesc = {};

	ID3D12Resource* resource = nullptr;

	heapProps.Type = heapType;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// TODO: set properly for multi-adaptor.
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	resDesc.Dimension = resourceDimention;
	resDesc.Width = size.X;
	resDesc.Height = size.Y;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = format;
	resDesc.Layout = (resourceDimention == D3D12_RESOURCE_DIMENSION_BUFFER ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN);
	resDesc.SampleDesc.Count = 1;

	auto hr =
		GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, resourceState, nullptr, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
	{
		SafeRelease(resource);
		return nullptr;
	}
	return resource;
}

ID3D12DescriptorHeap* GraphicsDX12::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12DescriptorHeap* heap = nullptr;

	heapDesc.NumDescriptors = 1;
	heapDesc.Type = heapType;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// TODO: set properly for multi-adaptor.
	heapDesc.NodeMask = 1;

	auto hr = GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));
	if (FAILED(hr))
	{
		SafeRelease(heap);
		return nullptr;
	}
	return heap;
}

} // namespace LLGI
