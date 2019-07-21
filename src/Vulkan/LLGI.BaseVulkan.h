
#pragma once

#include "../LLGI.Base.h"

#ifdef _WIN32
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <iostream>

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

uint32_t GetMemoryTypeIndex(vk::PhysicalDevice& phDevice, uint32_t bits, const vk::MemoryPropertyFlags& properties);

bool CreateDepthBuffer(vk::Image& image,
					   vk::ImageView view,
					   vk::DeviceMemory devMem,
					   vk::Device& device,
					   vk::PhysicalDevice& phDevice,
					   const Vec2I& size,
					   vk::Format format,
					   vk::CommandBuffer* commandBuffer);


class VulkanHelper
{
public:
    static const char* getResultName(VkResult result);

};

#define LLGI_VK_CHECK(f) \
{ \
    VkResult r = (f); \
	if (r != VK_SUCCESS) { \
        std::cerr << #f << "; VkResult:" << r << "(" << VulkanHelper::getResultName(r) << ")" << std::endl; \
		return false; \
	} \
}

} // namespace LLGI