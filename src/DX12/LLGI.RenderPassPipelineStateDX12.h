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

public:
	RenderPassPipelineStateDX12(GraphicsDX12* graphics) {}

	virtual ~RenderPassPipelineStateDX12() {}
};

struct RenderPassPipelineStateDX12Key
{
	bool isPresentMode;
	bool hasDepth;

	bool operator==(const RenderPassPipelineStateDX12Key& value) const
	{
		return (isPresentMode == value.isPresentMode && hasDepth == value.hasDepth);
	}

	struct Hash
	{
		typedef std::size_t result_type;

		std::size_t operator()(const RenderPassPipelineStateDX12Key& key) const
		{
			return std::hash<std::int32_t>()(std::hash<bool>()(key.hasDepth));
		}
	};
};

} // namespace LLGI
