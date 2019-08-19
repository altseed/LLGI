#include "LLGI.GraphicsVulkan.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.CommandListVulkan.h"
#include "LLGI.ConstantBufferVulkan.h"
#include "LLGI.IndexBufferVulkan.h"
#include "LLGI.PipelineStateVulkan.h"
#include "LLGI.ShaderVulkan.h"
#include "LLGI.SingleFrameMemoryPoolVulkan.h"
#include "LLGI.TextureVulkan.h"
#include "LLGI.VertexBufferVulkan.h"

namespace LLGI
{

RenderPassVulkan::RenderPassVulkan(GraphicsVulkan* graphics, bool isStrongRef) : graphics_(graphics), isStrongRef_(isStrongRef)
{
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}
}

RenderPassVulkan::~RenderPassVulkan()
{
	if (frameBuffer != nullptr)
	{
		graphics_->GetDevice().destroyFramebuffer(frameBuffer);
	}

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

bool RenderPassVulkan::Initialize(const vk::Image& imageColor,
								  const vk::Image& imageDepth,
								  const vk::ImageView& imageColorView,
								  const vk::ImageView& imageDepthView,
								  Vec2I imageSize,
								  vk::Format format)
{
	imageSize_ = imageSize;

	bool hasDepth = true;
	bool isPresentMode = true;

	this->renderPassPipelineState = graphics_->CreateRenderPassPipelineState(isPresentMode, hasDepth, format);

	std::array<vk::ImageView, 2> views;
	views[0] = imageColorView;
	views[1] = imageDepthView;

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPassPipelineState->GetRenderPass();
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(views.size());
	framebufferCreateInfo.pAttachments = views.data();
	framebufferCreateInfo.width = imageSize.X;
	framebufferCreateInfo.height = imageSize.Y;
	framebufferCreateInfo.layers = 1;

	frameBuffer = graphics_->GetDevice().createFramebuffer(framebufferCreateInfo);

	colorBuffers[0] = imageColor;
	depthBuffer = imageDepth;

	return true;
}

bool RenderPassVulkan::Initialize(const TextureVulkan** textures, int32_t textureCount, TextureVulkan* depthTexture)
{
	throw "Not inplemented";

	if (textureCount == 0)
		return false;

	for (int32_t i = 0; i < textureCount; i++)
	{
		auto texture = const_cast<TextureVulkan*>(textures[i]);
		SafeAddRef(texture);
		colorBufferPtrs[i] = CreateSharedPtr(texture);
	}

	if (depthTexture != nullptr)
	{
		SafeAddRef(depthTexture);
		depthBufferPtr = CreateSharedPtr(depthTexture);
	}
	else
	{
		depthBufferPtr.reset();
	}

	// TODO : make const
	imageSize_ = ((TextureVulkan*)textures[0])->GetSizeAs2D();

	vk::Format format = textures[0]->GetVulkanFormat();

	bool hasDepth = depthTexture != nullptr;
	bool isPresentMode = false;

	/*

	// TODO : MRT
	this->renderPassPipelineState = graphics_->CreateRenderPassPipelineState(isPresentMode, hasDepth, format);

	std::array<vk::ImageView, 2> views;
	views[0] = imageColorView;
	views[1] = imageDepthView;

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPassPipelineState->GetRenderPass();
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(views.size());
	framebufferCreateInfo.pAttachments = views.data();
	framebufferCreateInfo.width = imageSize.X;
	framebufferCreateInfo.height = imageSize.Y;
	framebufferCreateInfo.layers = 1;

	frameBuffer = graphics_->GetDevice().createFramebuffer(framebufferCreateInfo);

	colorBuffers[0] = imageColor;
	depthBuffer = imageDepth;

	return true;
	*/
}

Vec2I RenderPassVulkan::GetImageSize() const { return imageSize_; }

RenderPassPipelineState* RenderPassVulkan::CreateRenderPassPipelineState()
{
	auto ret = renderPassPipelineState.get();
	SafeAddRef(ret);
	return ret;
}

RenderPassPipelineStateVulkan::RenderPassPipelineStateVulkan(GraphicsVulkan* graphics) { graphics_ = graphics; }

RenderPassPipelineStateVulkan::~RenderPassPipelineStateVulkan()
{
	if (renderPass != nullptr)
	{
		graphics_->GetDevice().destroyRenderPass(renderPass);
	}
}

vk::RenderPass RenderPassPipelineStateVulkan::GetRenderPass() const { return renderPass; }

GraphicsVulkan::GraphicsVulkan(const vk::Device& device,
							   const vk::Queue& quque,
							   const vk::CommandPool& commandPool,
							   const vk::PhysicalDevice& pysicalDevice,
							   const PlatformView& platformView,
							   std::function<void(vk::CommandBuffer&)> addCommand,
							   std::function<void(PlatformStatus&)> getStatus)
	: vkDevice(device)
	, vkQueue(quque)
	, vkCmdPool(commandPool)
	, vkPysicalDevice(pysicalDevice)
	, addCommand_(addCommand)
	, getStatus_(getStatus)
{
	swapBufferCount_ = platformView.colors.size();

	for (size_t i = 0; i < static_cast<size_t>(swapBufferCount_); i++)
	{
		auto renderPass = std::make_shared<RenderPassVulkan>(this, false);
		renderPass->Initialize(platformView.colors[i],
							   platformView.depths[i],
							   platformView.colorViews[i],
							   platformView.depthViews[i],
							   platformView.imageSize,
							   platformView.format);
		renderPasses.push_back(renderPass);
	}

	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	defaultSampler = vkDevice.createSampler(samplerInfo);
}

GraphicsVulkan::~GraphicsVulkan()
{

	if (defaultSampler != nullptr)
	{
		vkDevice.destroySampler(defaultSampler);
	}
}

void GraphicsVulkan::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsVulkan::Execute(CommandList* commandList)
{
	auto commandList_ = static_cast<CommandListVulkan*>(commandList);
	addCommand_(commandList_->GetCommandBuffer());
}

void GraphicsVulkan::WaitFinish() { vkQueue.waitIdle(); }

RenderPass* GraphicsVulkan::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	PlatformStatus status;
	getStatus_(status);
	auto currentRenderPass = renderPasses[status.currentSwapBufferIndex];

	currentRenderPass->SetClearColor(clearColor);
	currentRenderPass->SetIsColorCleared(isColorCleared);
	currentRenderPass->SetIsDepthCleared(isDepthCleared);
	return currentRenderPass.get();
}

VertexBuffer* GraphicsVulkan::CreateVertexBuffer(int32_t size)
{
	auto obj = new VertexBufferVulkan();
	if (!obj->Initialize(this, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

IndexBuffer* GraphicsVulkan::CreateIndexBuffer(int32_t stride, int32_t count)
{

	auto obj = new IndexBufferVulkan();
	if (!obj->Initialize(this, stride, count))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

Shader* GraphicsVulkan::CreateShader(DataStructure* data, int32_t count)
{
	auto obj = new ShaderVulkan();
	if (!obj->Initialize(this, data, count))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

PipelineState* GraphicsVulkan::CreatePiplineState()
{

	auto pipelineState = new PipelineStateVulkan();

	if (pipelineState->Initialize(this))
	{
		return pipelineState;
	}

	SafeRelease(pipelineState);
	return nullptr;
}

SingleFrameMemoryPool* GraphicsVulkan::CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount)
{
	return new SingleFrameMemoryPoolVulkan(this, true, swapBufferCount_, constantBufferPoolSize, drawingCount);
}

CommandList* GraphicsVulkan::CreateCommandList(SingleFrameMemoryPool* memoryPool)
{
	auto mp = static_cast<SingleFrameMemoryPoolVulkan*>(memoryPool);

	auto commandList = new CommandListVulkan();
	if (commandList->Initialize(this, mp->GetDrawingCount()))
	{
		return commandList;
	}
	SafeRelease(commandList);
	return nullptr;
}

ConstantBuffer* GraphicsVulkan::CreateConstantBuffer(int32_t size)
{
	auto obj = new ConstantBufferVulkan();
	if (!obj->Initialize(this, size, ConstantBufferType::LongTime))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

RenderPass* GraphicsVulkan::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	throw "Not inplemented";

	if (textureCount > 1)
		throw "Not inplemented";

	auto renderPass = new RenderPassVulkan(this, true);
	if (!renderPass->Initialize((const TextureVulkan**)textures, textureCount, (TextureVulkan*)depthTexture))
	{
		SafeRelease(renderPass);
	}

	return renderPass;
}
Texture* GraphicsVulkan::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	auto obj = new TextureVulkan(this);
	if (!obj->Initialize(size, isRenderPass, isDepthBuffer))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

Texture* GraphicsVulkan::CreateTexture(uint64_t id) { throw "Not inplemented"; }

std::vector<uint8_t> GraphicsVulkan::CaptureRenderTarget(Texture* renderTarget)
{
	if (!renderTarget)
	{
		return std::vector<uint8_t>();
	}

	std::vector<uint8_t> result;
	VkDevice device = static_cast<VkDevice>(GetDevice());
	vkDeviceWaitIdle(device);

	auto texture = static_cast<TextureVulkan*>(renderTarget);
	auto width = texture->GetSizeAs2D().X;
	auto height = texture->GetSizeAs2D().Y;
	auto size = texture->GetMemorySize();
	auto image = static_cast<VkImage>(texture->GetImage());

	VulkanBuffer destBuffer;
	if (!destBuffer.Initialize(
			this, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		goto Exit;
	}

	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		// Swapchain image (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) -> copy source (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 0,
								 0,
								 nullptr,
								 0,
								 nullptr,
								 1,
								 &imageMemoryBarrier);
		}

		// Copy to destBuffer
		{
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = {0, 0, 0};
			region.imageExtent = {(uint32_t)width, (uint32_t)height, 1};
			vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destBuffer.GetNativeBuffer(), 1, &region);
		}

		// Undo layout
		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 0,
								 0,
								 nullptr,
								 0,
								 nullptr,
								 1,
								 &imageMemoryBarrier);
		}

		// Submit and Wait
		if (!EndSingleTimeCommands(commandBuffer))
		{
			goto Exit;
		}
	}

	// Blit
	{
		void* rawData = nullptr;
		vkMapMemory(device, destBuffer.GetNativeBufferMemory(), 0, destBuffer.GetSize(), 0, &rawData);
		result.resize(destBuffer.GetSize());
		memcpy(result.data(), rawData, result.size());
		vkUnmapMemory(device, destBuffer.GetNativeBufferMemory());
	}

Exit:
	destBuffer.Dispose();
	return result;
}

std::shared_ptr<RenderPassPipelineStateVulkan>
GraphicsVulkan::CreateRenderPassPipelineState(bool isPresentMode, bool hasDepth, vk::Format format)
{
	RenderPassPipelineStateVulkanKey key;
	key.isPresentMode = isPresentMode;
	key.hasDepth = hasDepth;
	key.format = format;

	// already?
	{
		auto it = renderPassPipelineStates.find(key);

		if (it != renderPassPipelineStates.end())
		{
			auto ret = it->second.lock();

			if (ret != nullptr)
				return ret;
		}
	}

	// settings
	std::array<vk::AttachmentDescription, 2> attachmentDescs;
	std::array<vk::AttachmentReference, 2> attachmentRefs;

	// color buffer
	attachmentDescs[0].format = format;
	attachmentDescs[0].samples = vk::SampleCountFlagBits::e1;

	// attachmentDescs[0].loadOp = vk::AttachmentLoadOp::eDontCare;

	// TODO : improve it
	attachmentDescs[0].loadOp = vk::AttachmentLoadOp::eClear;

	attachmentDescs[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescs[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDescs[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescs[0].initialLayout = vk::ImageLayout::eUndefined;

	if (isPresentMode)
	{
		attachmentDescs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	}
	else
	{
		attachmentDescs[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	// depth buffer
	if (hasDepth)
	{
		attachmentDescs[1].format = vk::Format::eD32SfloatS8Uint;
		attachmentDescs[1].samples = vk::SampleCountFlagBits::e1;

		// attachmentDescs[1].loadOp = vk::AttachmentLoadOp::eDontCare;
		// TODO : improve it
		attachmentDescs[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDescs[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescs[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescs[1].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachmentDescs[1].initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescs[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	vk::AttachmentReference& colorReference = attachmentRefs[0];
	colorReference.attachment = 0;
	colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference& depthReference = attachmentRefs[1];
	depthReference.attachment = 1;
	depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::array<vk::SubpassDescription, 1> subpasses;
	{
		vk::SubpassDescription& subpass = subpasses[0];
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &attachmentRefs[0];
		subpass.pDepthStencilAttachment = &attachmentRefs[1];
	}

	std::array<vk::SubpassDependency, 1> subpassDepends;
	{
		vk::SubpassDependency& dependency = subpassDepends[0];

		/*
		//monsho
		dependency.srcSubpass = 0;
		dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		*/

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcAccessMask = static_cast<vk::AccessFlagBits>(0);
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}

	{
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescs.size();
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.subpassCount = (uint32_t)subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();

		// based on official
		// renderPassInfo.dependencyCount = (uint32_t)subpassDepends.size();
		// renderPassInfo.pDependencies = subpassDepends.data();
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		auto renderPass = GetDevice().createRenderPass(renderPassInfo);
		if (renderPass == nullptr)
		{
			return nullptr;
		}

		std::shared_ptr<RenderPassPipelineStateVulkan> ret = std::make_shared<RenderPassPipelineStateVulkan>(this);
		ret->renderPass = renderPass;

		renderPassPipelineStates[key] = ret;

		return ret;
	}
}

int32_t GraphicsVulkan::GetSwapBufferCount() const { return swapBufferCount_; }

uint32_t GraphicsVulkan::GetMemoryTypeIndex(uint32_t bits, const vk::MemoryPropertyFlags& properties)
{
	return LLGI::GetMemoryTypeIndex(vkPysicalDevice, bits, properties);
}

VkCommandBuffer GraphicsVulkan::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = static_cast<VkCommandPool>(GetCommandPool());
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(static_cast<VkDevice>(GetDevice()), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

bool GraphicsVulkan::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	LLGI_VK_CHECK(vkQueueSubmit(static_cast<VkQueue>(vkQueue), 1, &submitInfo, VK_NULL_HANDLE));
	LLGI_VK_CHECK(vkQueueWaitIdle(static_cast<VkQueue>(vkQueue)));

	vkFreeCommandBuffers(static_cast<VkDevice>(GetDevice()), static_cast<VkCommandPool>(GetCommandPool()), 1, &commandBuffer);

	return true;
}

} // namespace LLGI