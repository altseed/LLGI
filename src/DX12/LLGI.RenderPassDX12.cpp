#include "LLGI.RenderPassDX12.h"
#include "LLGI.CommandListDX12.h"
#include "LLGI.DescriptorHeapDX12.h"
#include "LLGI.GraphicsDX12.h"
#include "LLGI.RenderPassPipelineStateDX12.h"
#include "LLGI.TextureDX12.h"

namespace LLGI
{

RenderPassDX12::RenderPassDX12(GraphicsDX12* graphics, bool isStrongRef) : graphics_(graphics), isStrongRef_(isStrongRef)
{
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}
}

RenderPassDX12 ::~RenderPassDX12()
{

	for (size_t i = 0; i < numRenderTarget_; i++)
	{
		if (renderTargets_[i].texture_ != nullptr)
			SafeRelease(renderTargets_[i].texture_);
	}
	renderTargets_.clear();

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

bool RenderPassDX12::Initialize() { return false; }

bool RenderPassDX12::Initialize(TextureDX12** textures, int numTextures, TextureDX12* depthTexture)
{
	if (textures[0]->Get() == nullptr)
		return false;

	isScreen_ = false;
	renderTargets_.resize(numTextures);
	numRenderTarget_ = numTextures;

	for (size_t i = 0; i < numTextures; i++)
	{
		renderTargets_[i].texture_ = textures[i];
		renderTargets_[i].renderPass_ = textures[i]->Get();
		SafeAddRef(renderTargets_[i].texture_);
	}

	auto size = textures[0]->GetSizeAs2D();
	screenWindowSize_.X = size.X;
	screenWindowSize_.Y = size.Y;

	return true;
}

Texture* RenderPassDX12::GetColorBuffer(int index) { return graphics_->GetScreenAsTexture(graphics_->GetSwapBuffer(index)); }

RenderPassPipelineState* RenderPassDX12::CreateRenderPassPipelineState()
{
	auto ret = renderPassPipelineState_.get();
	if (ret == nullptr)
	{
		renderPassPipelineState_ = graphics_->CreateRenderPassPipelineState(false /*TODO*/, false /*TODO*/, this);
		renderPassPipelineState_->SetRenderPass(this);
		ret = renderPassPipelineState_.get();
	}
	SafeAddRef(ret);
	return ret;
}

bool RenderPassDX12::CreateRenderTargetViews(CommandListDX12* commandList, DescriptorHeapDX12* rtDescriptorHeap)
{
	if (numRenderTarget_ == 0)
		return false;

	handleRTV_.resize(numRenderTarget_);

	for (int i = 0; i < numRenderTarget_; i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = renderTargets_[i].texture_->GetDXGIFormat();
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		auto cpuHandle = rtDescriptorHeap->GetCpuHandle();
		graphics_->GetDevice()->CreateRenderTargetView(renderTargets_[i].renderPass_, &desc, cpuHandle);
		handleRTV_[i] = cpuHandle;
		rtDescriptorHeap->IncrementCpuHandle(1);
		rtDescriptorHeap->IncrementGpuHandle(1);

		// memory barrior to make a rendertarget
		renderTargets_[i].texture_->ResourceBarrior(commandList->GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	return true;
}

bool RenderPassDX12::CreateScreenRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE handleRTV,
											  ID3D12Resource* renderPass,
											  const Color8& clearColor,
											  const bool isColorCleared,
											  const bool isDepthCleared,
											  const Vec2I windowSize)
{
	numRenderTarget_ = 1;
	handleRTV_.resize(1);
	renderTargets_.resize(1);

	renderTargets_[0].renderPass_ = renderPass;
	handleRTV_[0] = handleRTV;
	SetClearColor(clearColor);
	SetIsColorCleared(isColorCleared);
	SetIsDepthCleared(isDepthCleared);
	screenWindowSize_ = windowSize;

	return true;
}

} // namespace LLGI
