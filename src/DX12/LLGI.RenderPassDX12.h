#pragma once

#include "LLGI.BaseDX12.h"

namespace LLGI
{

class GraphicsDX12;
class RenderPassPipelineStateDX12;

class RenderPassDX12 : public RenderPass
{
private:
	GraphicsDX12* graphics_ = nullptr;
	bool isStrongRef_ = false;
	std::shared_ptr<RenderPassPipelineStateDX12> renderPassPipelineState;

public:
	Vec2I screenWindowSize;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV;
	ID3D12Resource* RenderPass;

	RenderPassDX12(GraphicsDX12* graphics, bool isStrongRef);
	virtual ~RenderPassDX12();

	RenderPassPipelineState* CreateRenderPassPipelineState() override;
};

} // namespace LLGI
