#pragma once

#include "../LLGI.Graphics.h"
#include "../utils/LLGI.FixedSizeVector.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.RenderPassVulkan.h"
#include <functional>
#include <unordered_map>

namespace LLGI
{

class RenderPassPipelineStateCacheVulkan : public ReferenceObject
{
private:
	std::unordered_map<RenderPassPipelineStateVulkanKey,
					   std::shared_ptr<RenderPassPipelineStateVulkan>,
					   RenderPassPipelineStateVulkanKey::Hash>
		renderPassPipelineStates_;

	vk::Device device_;
	ReferenceObject* owner_ = nullptr;

public:
	RenderPassPipelineStateCacheVulkan(vk::Device device, ReferenceObject* owner);
	virtual ~RenderPassPipelineStateCacheVulkan();

	RenderPassPipelineStateVulkan* Create(bool isPresentMode, bool hasDepth, const FixedSizeVector<vk::Format, 4>& formats);
};

} // namespace LLGI