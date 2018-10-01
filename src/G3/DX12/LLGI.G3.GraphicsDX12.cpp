#include "LLGI.G3.GraphicsDX12.h"
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

}

GraphicsDX12::~GraphicsDX12()
{

}

RenderTarget* GraphicsDX12::GetCurrentScreen()
{
	auto currentParam = getScreenFunc_();
	currentScreen.handleRTV = std::get<0>(currentParam);
	currentScreen.renderTarget = std::get<1>(currentParam);
	return &currentScreen;
}

CommandList* GraphicsDX12::CreateCommandList()
{
	auto obj = new CommandListDX12();
	if (obj->Initialize(this))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return nullptr;
}

ID3D12Device* GraphicsDX12::GetDevice()
{
	return device_;
}

}
}
