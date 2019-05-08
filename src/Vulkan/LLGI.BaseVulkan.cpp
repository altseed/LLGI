#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

Buffer::Buffer(GraphicsVulkan* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);
}

Buffer::~Buffer()
{
	if (buffer != nullptr)
	{
		graphics_->GetDevice().destroyBuffer(buffer);
		graphics_->GetDevice().freeMemory(devMem);
		buffer = nullptr;
	}
}

void SetImageLayout(vk::CommandBuffer cmdbuffer,
					vk::Image image,
					vk::ImageLayout oldImageLayout,
					vk::ImageLayout newImageLayout,
					vk::ImageSubresourceRange subresourceRange)
{
	vk::ImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// current layout

	if (oldImageLayout == vk::ImageLayout::ePreinitialized)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (oldImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	else if (oldImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;

	// next layout

	if (newImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (newImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead; // | imageMemoryBarrier.srcAccessMask;
	else if (newImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	if (imageMemoryBarrier.dstAccessMask == vk::AccessFlagBits::eTransferWrite)
	{
		cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
								  vk::PipelineStageFlagBits::eTransfer,
								  vk::DependencyFlags(),
								  nullptr,
								  nullptr,
								  imageMemoryBarrier);
	}
	else if (imageMemoryBarrier.dstAccessMask == vk::AccessFlagBits::eShaderRead)
	{
		cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
								  vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader,
								  vk::DependencyFlags(),
								  nullptr,
								  nullptr,
								  imageMemoryBarrier);
	}
	else
	{
		// Put barrier inside setup command buffer
		cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
								  vk::PipelineStageFlagBits::eTopOfPipe,
								  vk::DependencyFlags(),
								  nullptr,
								  nullptr,
								  imageMemoryBarrier);
	}
}

uint32_t GetMemoryTypeIndex(vk::PhysicalDevice& phDevice, uint32_t bits, const vk::MemoryPropertyFlags& properties)
{
	uint32_t result = 0;
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = phDevice.getMemoryProperties();
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((bits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		bits >>= 1;
	}

	assert(!"NOT found memory type.\n");
	return 0xffffffff;
}

bool CreateDepthBuffer(vk::Image& image,
					   vk::ImageView view,
					   vk::DeviceMemory devMem,
					   vk::Device& device,
					   vk::PhysicalDevice& phDevice,
					   const Vec2I& size,
					   vk::Format format,
					   vk::CommandBuffer* commandBuffer)
{
	// check a format whether specified format is supported
	vk::Format depthFormat = format;
	vk::FormatProperties formatProps = phDevice.getFormatProperties(depthFormat);
	if (!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment))
	{
		return false;
	}

	vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

	// create an image
	vk::ImageCreateInfo imageCreateInfo;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent = vk::Extent3D(size.X, size.Y, 1);
	imageCreateInfo.format = depthFormat;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	image = device.createImage(imageCreateInfo);

	// allocate memory
	vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(image);
	vk::MemoryAllocateInfo memAlloc;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryTypeIndex(phDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	devMem = device.allocateMemory(memAlloc);
	device.bindImageMemory(image, devMem, 0);

	// create view
	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;
	viewCreateInfo.format = depthFormat;
	viewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
	viewCreateInfo.subresourceRange.aspectMask = aspect;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.image = image;
	view = device.createImageView(viewCreateInfo);

	// change layout(nt needed?)

	if (commandBuffer != nullptr)
	{
		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = aspect;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		//SetImageBarrior(
		SetImageLayout(
			*commandBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, subresourceRange);
	}
}

} // namespace LLGI