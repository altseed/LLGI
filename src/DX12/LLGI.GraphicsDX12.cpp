#include "LLGI.GraphicsDX12.h"
#include "LLGI.CommandListDX12.h"
#include "LLGI.IndexBufferDX12.h"
#include "LLGI.PipelineStateDX12.h"
#include "LLGI.ShaderDX12.h"
#include "LLGI.VertexBufferDX12.h"

namespace LLGI
{

GraphicsDX12::GraphicsDX12(ID3D12Device* device,
						   std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc,
						   std::function<void()> waitFunc,
						   ID3D12CommandQueue* commandQueue)
	: device_(device), getScreenFunc_(getScreenFunc), waitFunc_(waitFunc), commandQueue_(commandQueue)
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

	// auto vertexView = D3D12_VERTEX_BUFFER_VIEW();
	// vertexView.BufferLocation = obj->GetGPUVirtualAddress();
	// vertexView.StrideInBytes = sizeof(Vertex3D);
	// vertexView.SizeInBytes = size * sizeof(Vertex3D);

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
	if (!obj->Initialize(this, commandAllocator_))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

PipelineState* GraphicsDX12::CreatePiplineState() { return new PipelineStateDX12(this); }

ID3D12Device* GraphicsDX12::GetDevice() { return device_; }

} // namespace LLGI
