
#pragma once

#include "../LLGI.Base.h"
#include <iostream>

#ifdef _WIN32
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#define LLGI_VK_CHECK(f)                                                                                                                   \
	{                                                                                                                                      \
		VkResult r = (f);                                                                                                                  \
		if (r != VK_SUCCESS)                                                                                                               \
		{                                                                                                                                  \
			std::cerr << #f << "; VkResult:" << r << "(" << VulkanHelper::getResultName(r) << ")" << std::endl;                            \
			return false;                                                                                                                  \
		}                                                                                                                                  \
	}

namespace LLGI
{

class GraphicsVulkan;
class PipelineStateVulkan;
class TextureVulkan;
class RenderPassVulkan;
class RenderPassPipelineStateCacheVulkan;

class VulkanHelper
{
public:
	static const char* getResultName(VkResult result);
};

class Buffer
{
	std::shared_ptr<GraphicsVulkan> graphics_;

public:
	vk::Buffer buffer_;
	vk::DeviceMemory devMem;

	Buffer(GraphicsVulkan* graphics);
	virtual ~Buffer();
};

class VulkanBuffer
{
public:
	VulkanBuffer();
	bool Initialize(GraphicsVulkan* graphics, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void Dispose();
	VkBuffer GetNativeBuffer() const { return nativeBuffer_; }
	VkDeviceMemory GetNativeBufferMemory() const { return nativeBufferMemory_; }
	VkDeviceSize GetSize() const { return size_; }

private:
	GraphicsVulkan* graphics_;
	VkBuffer nativeBuffer_;
	VkDeviceMemory nativeBufferMemory_;
	VkDeviceSize size_;
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

} // namespace LLGI