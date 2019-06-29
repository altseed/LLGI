#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"

namespace LLGI
{

class GraphicsDX12;
class TextureDX12;
class RenderPassPipelineStateDX12;

class RenderPassDX12 : public RenderPass
{
private:
	GraphicsDX12* graphics_ = nullptr;
	bool isStrongRef_ = false;
	bool isScreen_ = true;
	std::vector<TextureDX12*> textures_;
	std::shared_ptr<RenderPassPipelineStateDX12> renderPassPipelineState;

public:
	Vec2I screenWindowSize;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRtv_;
	ID3D12Resource* renderPass_ = nullptr;

	RenderPassDX12(GraphicsDX12* graphics, bool isStrongRef);
	virtual ~RenderPassDX12();

	bool Initialize();

	bool Initialize(TextureDX12** textures, int numTextures, TextureDX12* depthTexture);

	RenderPassPipelineState* CreateRenderPassPipelineState() override;

	RenderPassPipelineStateDX12* GetRenderPassPipelineState();

	const std::vector<TextureDX12*>& GetTextures() const { return textures_; }

	bool GetIsScreen() const { return isScreen_; }
};

} // namespace LLGI
