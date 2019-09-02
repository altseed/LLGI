#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"
#include <functional>
#include <unordered_map>

namespace LLGI
{

class GraphicsDX12;
class RenderPassDX12;

class RenderPassPipelineStateDX12 : public RenderPassPipelineState
{
private:
	GraphicsDX12* graphics_ = nullptr;
	RenderPassDX12* renderPass_;

public:
	RenderPassPipelineStateDX12(GraphicsDX12* graphics) : graphics_(graphics) {}
	RenderPassDX12* GetRenderPass() const { return renderPass_; }
	void SetRenderPass(RenderPassDX12* renderPass) { renderPass_ = renderPass; }

	virtual ~RenderPassPipelineStateDX12() {}
};

struct RenderPassPipelineStateDX12Key
{
	bool isPresentMode;
	bool hasDepth;
	RenderPassDX12* renderPass;

	bool operator==(const RenderPassPipelineStateDX12Key& value) const
	{
		return (isPresentMode == value.isPresentMode && hasDepth == value.hasDepth && renderPass == value.renderPass);
	}

	struct Hash
	{
		typedef std::size_t result_type;

		std::size_t operator()(const RenderPassPipelineStateDX12Key& key) const { return std::hash<bool>()(key.hasDepth); }
	};
};

} // namespace LLGI
