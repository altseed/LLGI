
#include "LLGI.G3.CommandListDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

namespace LLGI
{
namespace G3
{

CommandListDX12::CommandListDX12() {}

CommandListDX12::~CommandListDX12()
{
	SafeRelease(commandAllocator_);
	SafeRelease(commandList_);
}

bool CommandListDX12::Initialize(GraphicsDX12* graphics, ID3D12CommandAllocator* commandAllocator)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);
	SafeAddRef(commandAllocator);
	commandAllocator_ = CreateSharedPtr(commandAllocator);

	HRESULT hr;
	hr = graphics_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	commandList_->Close();

	return true;

FAILED_EXIT:;
	SafeRelease(commandList_);
	SafeRelease(commandAllocator);
	SafeRelease(graphics);
	return false;
}

void CommandListDX12::Begin() { commandList_->Reset(commandAllocator_.get(), nullptr); }

void CommandListDX12::End() { commandList_->Close(); }

void CommandListDX12::Clear(const Color8& color)
{
	auto rt = renderPass_;
	if (rt == nullptr)
		return;

	float color_[] = {color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f};

	commandList_->ClearRenderTargetView(rt->handleRTV, color_, 0, nullptr);
}

void CommandListDX12::BeginRenderPass(RenderPass* renderPass)
{
	SafeAddRef(renderPass);
	renderPass_ = CreateSharedPtr((RenderPassDX12*)renderPass);

	if (renderPass != nullptr)
	{
		// Reset scissor
		D3D12_RECT rect;
		rect.top = 0;
		rect.left = 0;
		rect.right = renderPass_->screenWindowSize.X;
		rect.bottom = renderPass_->screenWindowSize.Y;
		commandList_->RSSetScissorRects(1, &rect);

		// Clear color
		if (renderPass_->GetIsColorCleared())
		{
			Clear(renderPass_->GetClearColor());
		}
	}
}

void CommandListDX12::EndRenderPass() { renderPass_.reset(); }

} // namespace G3
} // namespace LLGI
