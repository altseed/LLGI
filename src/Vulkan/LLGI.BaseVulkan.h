
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
class ShaderVulkan;

class Buffer
{
	GraphicsVulkan* graphics_ = nullptr;
	bool hasStrongRef_ = false;

public:
	vk::Buffer buffer;
	vk::DeviceMemory devMem;

	Buffer(GraphicsVulkan* graphics, bool hasStrongRef = true);
	virtual ~Buffer();
};

void SetImageLayout(vk::CommandBuffer cmdbuffer,
					vk::Image image,
					vk::ImageLayout oldImageLayout,
					vk::ImageLayout newImageLayout,
					vk::ImageSubresourceRange subresourceRange);

} // namespace LLGI