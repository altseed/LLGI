#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"

namespace LLGI
{

class GraphicsDX12;
class TextureDX12;
class RenderPassPipelineStateDX12;
class RenderTargetDX12;
class CommandListDX12;
class DescriptorHeapDX12;

class RenderPassDX12 : public RenderPass
{
private:
	GraphicsDX12* graphics_ = nullptr;
	bool isStrongRef_ = false;
	bool isScreen_ = true;

	std::vector<RenderTargetDX12> renderTargets_;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handleRTV_;
	int32_t numRenderTarget_;
	Vec2I screenWindowSize_;

public:
	RenderPassDX12(GraphicsDX12* graphics, bool isStrongRef);
	virtual ~RenderPassDX12();

	bool Initialize();
	bool Initialize(TextureDX12** textures, int numTextures, TextureDX12* depthTexture);

	// RenderPassPipelineState* CreateRenderPassPipelineState() override;

	RenderPassPipelineStateDX12* GetRenderPassPipelineState(int idx);

	const D3D12_CPU_DESCRIPTOR_HANDLE* GetHandleRTV() const { return handleRTV_.data(); }
	const RenderTargetDX12* GetRenderTarget(int idx) const { return &renderTargets_[idx]; }
	int32_t GetCount() const { return numRenderTarget_; }
	bool GetIsScreen() const { return isScreen_; }
	Vec2I GetScreenWindowSize() const { return screenWindowSize_; }

	bool CreateRenderTargetViews(CommandListDX12* commandList, DescriptorHeapDX12* rtDescriptorHeap);
	bool CreateScreenRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE handleRTV,
								  ID3D12Resource* renderPass,
								  const Color8& clearColor,
								  const bool isColorCleared,
								  const bool isDepthCleared,
								  const Vec2I windowSize);
};

class RenderTargetDX12
{
public:
	TextureDX12* texture_ = nullptr;
	ID3D12Resource* renderPass_ = nullptr;
	RenderPassPipelineStateDX12* renderPassPipelineState_ = nullptr;

	RenderTargetDX12() {}
};
} // namespace LLGI
