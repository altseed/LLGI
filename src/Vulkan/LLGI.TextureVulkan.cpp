
#include "LLGI.TextureVulkan.h"

namespace LLGI
{

void TextureVulkan::ResetImageLayouts(int32_t count, vk::ImageLayout layout)
{
	imageLayouts_.resize(count);

	for (int32_t i = 0; i < count; i++)
	{
		imageLayouts_[i] = layout;
	}
}

TextureVulkan::TextureVulkan() {}

TextureVulkan::~TextureVulkan()
{
	if (view_ && type_ != TextureType::Screen)
	{
		device_.destroyImageView(view_);
		view_ = nullptr;
	}

	if (image_)
	{
		if (type_ != TextureType::Screen && !isExternalResource_)
		{
			device_.destroyImage(image_);
			device_.freeMemory(devMem_);
			image_ = nullptr;
		}
	}

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}

	SafeRelease(owner_);
}

bool TextureVulkan::Initialize(GraphicsVulkan* graphics, bool isStrongRef, const TextureParameter& parameter)
{
	if (parameter.Dimension < 2)
	{
		return false;
	}

	graphics_ = graphics;
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}

	parameter_ = parameter;

	type_ = TextureType::Color;
	vk::ImageType dimension = vk::ImageType::e2D;

	if (parameter.Dimension == 2)
	{
		dimension = vk::ImageType::e2D;
	}
	else if (parameter.Dimension == 3)
	{
		dimension = vk::ImageType::e3D;
	}

	auto vkFormat = (vk::Format)VulkanHelper::TextureFormatToVkFormat(parameter.Format);
	format_ = parameter.Format;

	vk::ImageUsageFlags resourceFlag = {};

	if (IsDepthFormat(parameter.Format))
	{
		resourceFlag = resourceFlag | vk::ImageUsageFlagBits::eDepthStencilAttachment;
		type_ = TextureType::Depth;
	}
	else
	{
		resourceFlag =
			resourceFlag | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
	}

	if ((parameter.Usage & TextureUsageType::RenderTarget) != TextureUsageType::NoneFlag)
	{
		resourceFlag = resourceFlag | vk::ImageUsageFlagBits::eColorAttachment;
		type_ = TextureType::Render;
	}

	samplingCount_ = parameter.SampleCount;

	int mipmapCount = parameter.MipLevelCount;

	// check whether is mipmap enabled?
	auto properties = graphics_->GetPysicalDevice().getFormatProperties((vk::Format)vkFormat);
	if (!(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		mipmapCount = 1;
	}

	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics_));

	auto isArray = (parameter.Usage & TextureUsageType::Array) != TextureUsageType::NoneFlag;
	// image
	vk::ImageCreateInfo imageCreateInfo;

	imageCreateInfo.imageType = dimension;
	imageCreateInfo.extent.width = parameter.Size.X;
	imageCreateInfo.extent.height = parameter.Size.Y;
	imageCreateInfo.extent.depth = isArray ? 1 : parameter.Size.Z;
	imageCreateInfo.mipLevels = mipmapCount;
	imageCreateInfo.arrayLayers = isArray ? parameter.Size.Z : 1;
	imageCreateInfo.format = vkFormat;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	if (type_ == TextureType::Render)
	{
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst |
								vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
	}
	else
	{
		imageCreateInfo.usage =
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
	}

	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = (vk::SampleCountFlagBits)samplingCount_;
	imageCreateInfo.flags = (vk::ImageCreateFlagBits)0;

	image_ = graphics_->GetDevice().createImage(imageCreateInfo);

	// get device
	auto device = graphics_->GetDevice();

	// calculate size
	memorySize = GetTextureMemorySize(format_, parameter.Size);

	// create a buffer on cpu
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = memorySize;
		bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(bufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		cpuBuf->Attach(buffer, devMem);
	}

	// create a buffer on gpu
	{
		vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(image_);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		devMem_ = device.allocateMemory(memAlloc);
		graphics_->GetDevice().bindImageMemory(image_, devMem_, 0);
	}

	// create a texture view
	{
		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.image = image_;

		if (parameter.Dimension == 3)
		{
			imageViewInfo.viewType = vk::ImageViewType::e3D;
		}
		else if (isArray)
		{
			imageViewInfo.viewType = vk::ImageViewType::e2DArray;
		}
		else
		{
			imageViewInfo.viewType = vk::ImageViewType::e2D;
		}

		imageViewInfo.format = vkFormat;
		imageViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = mipmapCount;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = isArray ? parameter_.Size.Z : 1;
		subresourceRange_ = imageViewInfo.subresourceRange;
		view_ = device.createImageView(imageViewInfo);
	}

	textureSize = parameter.Size;
	mipmapCount_ = mipmapCount;
	vkTextureFormat_ = imageCreateInfo.format;
	format_ = VulkanHelper::VkFormatToTextureFormat(static_cast<VkFormat>(vkTextureFormat_));
	device_ = graphics_->GetDevice();

	ResetImageLayouts(mipmapCount_, imageCreateInfo.initialLayout);

	vk::CommandBufferAllocateInfo cmdBufInfo;
	cmdBufInfo.commandPool = graphics_->GetCommandPool();
	cmdBufInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdBufInfo.commandBufferCount = 1;
	auto cmdBuffers = graphics_->GetDevice().allocateCommandBuffersUnique(cmdBufInfo);

	// a texture state must starts from undefined, so the states must be changed with a command buffer
	vk::CommandBufferBeginInfo cmdBufferBeginInfo;
	cmdBuffers[0]->begin(cmdBufferBeginInfo);
	ResourceBarrior(cmdBuffers[0].get(), vk::ImageLayout::eShaderReadOnlyOptimal);
	cmdBuffers[0]->end();
	std::array<vk::SubmitInfo, 1> submitInfos;
	submitInfos[0].commandBufferCount = 1;
	submitInfos[0].pCommandBuffers = &(cmdBuffers[0].get());

	graphics_->GetQueue().submit(static_cast<uint32_t>(submitInfos.size()), submitInfos.data(), vk::Fence());
	graphics_->GetQueue().waitIdle();

	return true;
}

bool TextureVulkan::InitializeAsScreen(const vk::Image& image, const vk::ImageView& imageVew, vk::Format format, const Vec2I& size)
{
	type_ = TextureType::Screen;

	this->image_ = image;
	this->view_ = imageVew;
	vkTextureFormat_ = format;
	textureSize = {size.X, size.Y, 1};
	format_ = VulkanHelper::VkFormatToTextureFormat(static_cast<VkFormat>(vkTextureFormat_));
	memorySize = GetTextureMemorySize(format_, {size.X, size.Y, 1});

	subresourceRange_.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange_.baseArrayLayer = 0;
	subresourceRange_.levelCount = 1;
	subresourceRange_.baseMipLevel = 0;
	subresourceRange_.layerCount = 1;

	isExternalResource_ = true;

	mipmapCount_ = 1;
	ResetImageLayouts(mipmapCount_, vk::ImageLayout::eUndefined);

	return true;
}

bool TextureVulkan::InitializeAsDepthStencil(
	vk::Device device, vk::PhysicalDevice physicalDevice, const Vec2I& size, vk::Format format, int samplingCount, ReferenceObject* owner)
{
	type_ = TextureType::Depth;
	textureSize = {size.X, size.Y, 1};

	owner_ = owner;
	SafeAddRef(owner_);
	device_ = device;

	samplingCount_ = samplingCount;

	// check a format whether specified format is supported
	vk::Format depthFormat = format;
	format_ = VulkanHelper::VkFormatToTextureFormat(static_cast<VkFormat>(format));

	vk::FormatProperties formatProps = physicalDevice.getFormatProperties(depthFormat);
	if (!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment))
	{
		throw "Invalid formatProps";
	}

	vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth;
	if (HasStencil(format_))
	{
		aspect = aspect | vk::ImageAspectFlagBits::eStencil;
	}

	// create an image
	vk::ImageCreateInfo imageCreateInfo;
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent = vk::Extent3D(size.X, size.Y, 1);
	imageCreateInfo.format = depthFormat;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = (vk::SampleCountFlagBits)samplingCount_;
	imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment /* | vk::ImageUsageFlagBits::eSampled*/;
	image_ = device.createImage(imageCreateInfo);

	// allocate memory
	vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(image_);
	vk::MemoryAllocateInfo memAlloc;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryTypeIndex(physicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	devMem_ = device.allocateMemory(memAlloc);
	device.bindImageMemory(image_, devMem_, 0);

	// create view
	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;
	viewCreateInfo.format = depthFormat;
	viewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
	viewCreateInfo.subresourceRange.aspectMask = aspect;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.image = image_;
	view_ = device.createImageView(viewCreateInfo);

	subresourceRange_ = viewCreateInfo.subresourceRange;
	vkTextureFormat_ = depthFormat;

	mipmapCount_ = 1;
	ResetImageLayouts(mipmapCount_, imageCreateInfo.initialLayout);

	return true;
}

bool TextureVulkan::InitializeAsExternal(vk::Device device, const VulkanImageInfo& info, ReferenceObject* owner)
{
	type_ = TextureType::Color;
	textureSize = Vec3I{0, 0, 0};
	device_ = device;

	owner_ = owner;
	SafeAddRef(owner_);

	image_ = static_cast<vk::Image>(info.image);

	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;
	viewCreateInfo.format = (vk::Format)info.format;
	viewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
	viewCreateInfo.subresourceRange.aspectMask = (vk::ImageAspectFlagBits)info.aspect;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.image = image_;
	view_ = device.createImageView(viewCreateInfo);

	if (!view_)
	{
		return false;
	}

	mipmapCount_ = 1;
	ResetImageLayouts(mipmapCount_, vk::ImageLayout::eUndefined);

	isExternalResource_ = true;

	return true;
}

void* TextureVulkan::Lock()
{
	if (graphics_ == nullptr)
		return nullptr;

	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem(), 0, memorySize, vk::MemoryMapFlags());
	return data;
}

void TextureVulkan::Unlock()
{
	if (graphics_ == nullptr)
	{
		return;
	}

	graphics_->GetDevice().unmapMemory(cpuBuf->devMem());

	// copy buffer
	vk::CommandBufferAllocateInfo cmdBufInfo;
	cmdBufInfo.commandPool = graphics_->GetCommandPool();
	cmdBufInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdBufInfo.commandBufferCount = 1;
	vk::CommandBuffer copyCommandBuffer = graphics_->GetDevice().allocateCommandBuffers(cmdBufInfo)[0];

	vk::CommandBufferBeginInfo cmdBufferBeginInfo;

	copyCommandBuffer.begin(cmdBufferBeginInfo);

	auto isArray = (parameter_.Usage & TextureUsageType::Array) != TextureUsageType::NoneFlag;

	vk::BufferImageCopy imageBufferCopy;

	imageBufferCopy.bufferOffset = 0;
	imageBufferCopy.bufferRowLength = 0;
	imageBufferCopy.bufferImageHeight = 0;

	imageBufferCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageBufferCopy.imageSubresource.mipLevel = 0;
	imageBufferCopy.imageSubresource.baseArrayLayer = 0;
	imageBufferCopy.imageSubresource.layerCount = isArray ? parameter_.Size.Z : 1;

	imageBufferCopy.imageOffset = vk::Offset3D(0, 0, 0);
	imageBufferCopy.imageExtent =
		vk::Extent3D(static_cast<uint32_t>(GetSizeAs2D().X), static_cast<uint32_t>(GetSizeAs2D().Y), isArray ? 1 : parameter_.Size.Z);

	vk::ImageSubresourceRange colorSubRange;
	colorSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	colorSubRange.levelCount = 1;
	colorSubRange.layerCount = isArray ? parameter_.Size.Z : 1;

	vk::ImageLayout imageLayout = vk::ImageLayout::eTransferDstOptimal;
	ResourceBarrior(copyCommandBuffer, imageLayout);
	copyCommandBuffer.copyBufferToImage(cpuBuf->buffer(), image_, imageLayout, imageBufferCopy);
	ResourceBarrior(copyCommandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
	copyCommandBuffer.end();

	// submit and wait to execute command
	std::array<vk::SubmitInfo, 1> copySubmitInfos;
	copySubmitInfos[0].commandBufferCount = 1;
	copySubmitInfos[0].pCommandBuffers = &copyCommandBuffer;

	graphics_->GetQueue().submit(static_cast<uint32_t>(copySubmitInfos.size()), copySubmitInfos.data(), vk::Fence());
	graphics_->GetQueue().waitIdle();

	graphics_->GetDevice().freeCommandBuffers(graphics_->GetCommandPool(), copyCommandBuffer);
}

Vec2I TextureVulkan::GetSizeAs2D() const { return {textureSize.X, textureSize.Y}; }

std::vector<vk::ImageLayout> TextureVulkan::GetImageLayouts() const { return imageLayouts_; }

void TextureVulkan::ChangeImageLayout(const vk::ImageLayout& imageLayout)
{
	for (int32_t i = 0; i < mipmapCount_; i++)
	{
		ChangeImageLayout(i, imageLayout);
	}
}

void TextureVulkan::ChangeImageLayout(int32_t mipLevel, const vk::ImageLayout& imageLayout) { imageLayouts_[mipLevel] = imageLayout; }

void TextureVulkan::ResourceBarrior(vk::CommandBuffer& commandBuffer, const vk::ImageLayout& imageLayout)
{
	for (int32_t i = 0; i < mipmapCount_; i++)
	{
		ResourceBarrior(i, commandBuffer, imageLayout);
	}
}

void TextureVulkan::ResourceBarrior(int32_t mipLevel, vk::CommandBuffer& commandBuffer, const vk::ImageLayout& imageLayout)
{
	if (imageLayouts_[mipLevel] == imageLayout)
	{
		return;
	}

	auto subresourceRange = subresourceRange_;
	subresourceRange.baseMipLevel = mipLevel;
	subresourceRange.levelCount = 1;
	SetImageLayout(commandBuffer, image_, imageLayouts_[mipLevel], imageLayout, subresourceRange);
	ChangeImageLayout(mipLevel, imageLayout);
}

} // namespace LLGI
