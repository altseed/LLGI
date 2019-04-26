
#pragma once

#include "../LLGI.Base.h"

#ifdef _WIN32
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace LLGI
{

class GraphicsVulkan;
class PipelineStateVulkan;

class Buffer
{
	std::shared_ptr<GraphicsVulkan> graphics_;

public:
	vk::Buffer buffer;
	vk::DeviceMemory devMem;

	Buffer(GraphicsVulkan* graphics);
	virtual ~Buffer();
};

void SetImageLayout(vk::CommandBuffer cmdbuffer,
					vk::Image image,
					vk::ImageLayout oldImageLayout,
					vk::ImageLayout newImageLayout,
					vk::ImageSubresourceRange subresourceRange);

} // namespace LLGI