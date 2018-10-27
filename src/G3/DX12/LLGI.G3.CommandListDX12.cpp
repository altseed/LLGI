
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

void CommandListDX12::Clear(const Color8& color)
{
	auto rt = renderPass_;
	if (rt == nullptr) return;

	float color_[] = { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f };

	commandList->ClearRenderTargetView(rt->handleRTV, color_, 0, nullptr);
}

void CommandListDX12::BeginRenderPass(RenderPass* renderPass)
{
	SafeAddRef(renderPass);
	renderPass_ = CreateSharedPtr((RenderPassDX12*)renderPass);
}

void CommandListDX12::EndRenderPass()
{
	renderPass_.reset();
}

}
}