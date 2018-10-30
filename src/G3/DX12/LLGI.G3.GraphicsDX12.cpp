#include "LLGI.G3.GraphicsDX12.h"
#include "LLGI.G3.VertexBufferDX12.h"
#include "LLGI.G3.CommandListDX12.h"

namespace LLGI
{
namespace G3
{

GraphicsDX12::GraphicsDX12(ID3D12Device* device, std::function<std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc, ID3D12CommandQueue* commandQueue)
	: device_(device)
	, getScreenFunc_(getScreenFunc)
	, commandQueue_(commandQueue)
{
	SafeAddRef(device_);
	SafeAddRef(commandQueue_);
}

GraphicsDX12::~GraphicsDX12()
{
	SafeRelease(device_);
	SafeRelease(commandQueue_);
}

void GraphicsDX12::Execute(CommandList* commandList)
{
	auto cl = (CommandListDX12*)commandList;
	auto cl_internal = cl->GetCommandList();
	commandQueue_->ExecuteCommandLists(1, (ID3D12CommandList**)(&cl_internal));
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

	//auto vertexView = D3D12_VERTEX_BUFFER_VIEW();
	//vertexView.BufferLocation = obj->GetGPUVirtualAddress();
	//vertexView.StrideInBytes = sizeof(Vertex3D);
	//vertexView.SizeInBytes = size * sizeof(Vertex3D);

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

ID3D12Device* GraphicsDX12::GetDevice()
{
	return device_;
}

}
}
