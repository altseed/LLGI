
#include "LLGI.TextureVulkan.h"

namespace LLGI
{

TextureVulkan::TextureVulkan(GraphicsVulkan* graphics, bool isStrongRef) : graphics_(graphics), isStrongRef_(isStrongRef)
{
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}
}

TextureVulkan::~TextureVulkan()
{
	if (image_)
	{
		if (!isExternalResource_)
		{
			graphics_->GetDevice().destroyImageView(view);
			graphics_->GetDevice().destroyImage(image_);
			graphics_->GetDevice().freeMemory(devMem);

			image_ = nullptr;
			view = nullptr;
		}
	}

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

bool TextureVulkan::Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	if (isRenderPass)
		throw "Not implemented";

	if (isDepthBuffer)
		throw "Not implemented";

	if (isDepthBuffer)
	{
		isDepthBuffer_ = isDepthBuffer;
		// CreateDepthBuffer(this->image, this->view, this->devMem, graphics_->GetDevice(), ...)
	}

	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics_));

	vk::Format format = vk::Format::eR8G8B8A8Unorm;

	// image
	vk::ImageCreateInfo imageCreateInfo;

	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = size.X;
	imageCreateInfo.extent.height = size.Y;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	if (isRenderPass)
	{
		isRenderPass_ = isRenderPass;
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
	}
	else
	{
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	}

	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.flags = (vk::ImageCreateFlagBits)0;

	image_ = graphics_->GetDevice().createImage(imageCreateInfo);

	// get device
	auto& device = graphics_->GetDevice();

	// calculate size
	memorySize = size.X * size.Y * 4;

	// create a buffer on cpu
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = memorySize;
		bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		cpuBuf->buffer_ = graphics_->GetDevice().createBuffer(bufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(cpuBuf->buffer_);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		cpuBuf->devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(cpuBuf->buffer_, cpuBuf->devMem, 0);
	}

	// create a buffer on gpu
	{
		vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(image_);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		devMem = device.allocateMemory(memAlloc);
		graphics_->GetDevice().bindImageMemory(image_, devMem, 0);
	}

	// create a texture view
	{
		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.image = image_;
		imageViewInfo.viewType = vk::ImageViewType::e2D;
		imageViewInfo.format = format;
		imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		view = device.createImageView(imageViewInfo);
	}

	textureSize = size;
	vkTextureFormat = imageCreateInfo.format;

	return true;
}

bool TextureVulkan::Initialize(const vk::Image& image, const vk::ImageView& imageVew, vk::Format format, const Vec2I& size)
{
	this->image_ = image;
	this->view = imageVew;
	vkTextureFormat = format;
	textureSize = size;
	memorySize = size.X * size.Y * 4; // TODO: format
	isExternalResource_ = true;
	return true;
}

void* TextureVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem, 0, memorySize, vk::MemoryMapFlags());
	return data;
}

void TextureVulkan::Unlock()
{
	graphics_->GetDevice().unmapMemory(cpuBuf->devMem);

	// copy buffer
	vk::CommandBufferAllocateInfo cmdBufInfo;
	cmdBufInfo.commandPool = graphics_->GetCommandPool();
	cmdBufInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdBufInfo.commandBufferCount = 1;
	vk::CommandBuffer copyCommandBuffer = graphics_->GetDevice().allocateCommandBuffers(cmdBufInfo)[0];

	vk::CommandBufferBeginInfo cmdBufferBeginInfo;

	copyCommandBuffer.begin(cmdBufferBeginInfo);

	vk::ImageLayout imageLayout = vk::ImageLayout::eTransferDstOptimal;
	vk::BufferImageCopy imageBufferCopy;

	imageBufferCopy.bufferOffset = 0;
	imageBufferCopy.bufferRowLength = 0;
	imageBufferCopy.bufferImageHeight = 0;

	imageBufferCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageBufferCopy.imageSubresource.mipLevel = 0;
	imageBufferCopy.imageSubresource.baseArrayLayer = 0;
	imageBufferCopy.imageSubresource.layerCount = 1;

	imageBufferCopy.imageOffset = {0, 0, 0};
	imageBufferCopy.imageExtent = {static_cast<uint32_t>(GetSizeAs2D().X), static_cast<uint32_t>(GetSizeAs2D().Y), 1};

	vk::ImageSubresourceRange colorSubRange;
	colorSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	colorSubRange.levelCount = 1;
	colorSubRange.layerCount = 1;

	SetImageLayout(copyCommandBuffer, image_, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, colorSubRange);

	copyCommandBuffer.copyBufferToImage(cpuBuf->buffer_, image_, imageLayout, imageBufferCopy);

	SetImageLayout(copyCommandBuffer, image_, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, colorSubRange);

	copyCommandBuffer.end();

	// submit and wait to execute command
	std::array<vk::SubmitInfo, 1> copySubmitInfos;
	copySubmitInfos[0].commandBufferCount = 1;
	copySubmitInfos[0].pCommandBuffers = &copyCommandBuffer;

	graphics_->GetQueue().submit(copySubmitInfos.size(), copySubmitInfos.data(), vk::Fence());
	graphics_->GetQueue().waitIdle();

	graphics_->GetDevice().freeCommandBuffers(graphics_->GetCommandPool(), copyCommandBuffer);
}

Vec2I TextureVulkan::GetSizeAs2D() { return textureSize; }

bool TextureVulkan::IsRenderTexture() const
{
	throw "Not inplemented";
	return isRenderPass_;
}

bool TextureVulkan::IsDepthTexture() const
{
	throw "Not inplemented";
	return isDepthBuffer_;
}

} // namespace LLGI
