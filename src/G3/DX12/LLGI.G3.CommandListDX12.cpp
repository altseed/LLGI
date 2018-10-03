
#include "LLGI.G3.CommandListDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

namespace LLGI
{
namespace G3
{

CommandListDX12::CommandListDX12()
{

}

CommandListDX12::~CommandListDX12()
{
	SafeRelease(commandAllocator);
	SafeRelease(commandList);
}

bool CommandListDX12::Initialize(GraphicsDX12* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	HRESULT hr;

	hr = graphics_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = graphics_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	commandList->Close();

	return true;
FAILED_EXIT:;
	SafeRelease(commandAllocator);
	SafeRelease(commandList);
	return false;
}

void CommandListDX12::Begin()
{
	commandList->Reset(commandAllocator, nullptr);
}

void CommandListDX12::End()
{
	commandList->Close();
}

void CommandListDX12::Clear(RenderTarget* renderTarget, const Color8& color)
{
	auto rt = (RenderTargetDX12*)renderTarget;

	float color_[] = { 1.0, 1.0, 1.0, 1.0 };

	commandList->ClearRenderTargetView(rt->handleRTV, color_, 0, nullptr);
}

}
}