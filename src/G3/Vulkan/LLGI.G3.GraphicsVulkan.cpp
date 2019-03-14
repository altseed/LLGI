#include "LLGI.G3.GraphicsVulkan.h"

namespace LLGI
{
namespace G3
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
	if (renderPass != nullptr)
	{
		graphics_->GetDevice().destroyRenderPass(renderPass);
	}

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

	// settings
	std::array<vk::AttachmentDescription, 2> attachmentDescs;
	std::array<vk::AttachmentReference, 2> attachmentRefs;

	// color buffer
	attachmentDescs[0].format = format;
	attachmentDescs[0].loadOp = vk::AttachmentLoadOp::eDontCare;
	// attachmentDescs[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescs[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescs[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDescs[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	// attachmentDescs[0].initialLayout = vk::ImageLayout::eUndefined;
	// attachmentDescs[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// depth buffer
	attachmentDescs[1].format = vk::Format::eD32SfloatS8Uint;
	attachmentDescs[1].loadOp = vk::AttachmentLoadOp::eDontCare;
	// attachmentDescs[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescs[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescs[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachmentDescs[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	// attachmentDescs[1].initialLayout = vk::ImageLayout::eUndefined;
	// attachmentDescs[1].finalLayout = vk::ImageLayout::eUndefined;

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
		dependency.srcSubpass = 0;
		dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	}

	{
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescs.size();
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.subpassCount = (uint32_t)subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = (uint32_t)subpassDepends.size();
		renderPassInfo.pDependencies = subpassDepends.data();
		renderPass = graphics_->GetDevice().createRenderPass(renderPassInfo);
		if (renderPass == nullptr)
		{
			return false;
		}
	}

	std::array<vk::ImageView, 2> views;
	views[0] = imageColorView;
	views[1] = imageDepthView;

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPass;
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

Vec2I RenderPassVulkan::GetImageSize() const { return imageSize_; }

GraphicsVulkan::GraphicsVulkan(const vk::Device& device,
							   const vk::CommandPool& commandPool,
							   const PlatformView& platformView,
							   std::function<void(PlatformStatus&)> getStatus)
	: vkDevice(device), vkCmdPool(commandPool), getStatus_(getStatus)
{
	swapBufferCount_ = platformView.colors.size();

	for (size_t i = 0; i < swapBufferCount_; i++)
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
}

GraphicsVulkan::~GraphicsVulkan() {}

void GraphicsVulkan::NewFrame()
{
	currentSwapBufferIndex = (currentSwapBufferIndex + 1) % swapBufferCount_;

	PlatformStatus status;
	getStatus_(status);

	assert(currentSwapBufferIndex != status.currentSwapBufferIndex);
}

void GraphicsVulkan::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsVulkan::Execute(CommandList* commandList) { throw "Not inplemented"; }

void GraphicsVulkan::WaitFinish() { throw "Not inplemented"; }

RenderPass* GraphicsVulkan::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	auto currentRenderPass = renderPasses[currentSwapBufferIndex];

	currentRenderPass->SetClearColor(clearColor);
	currentRenderPass->SetIsColorCleared(isColorCleared);
	currentRenderPass->SetIsDepthCleared(isDepthCleared);
	return currentRenderPass.get();
}

VertexBuffer* GraphicsVulkan::CreateVertexBuffer(int32_t size) { throw "Not inplemented"; }

IndexBuffer* GraphicsVulkan::CreateIndexBuffer(int32_t stride, int32_t count) { throw "Not inplemented"; }

Shader* GraphicsVulkan::CreateShader(DataStructure* data, int32_t count) { throw "Not inplemented"; }

PipelineState* GraphicsVulkan::CreatePiplineState() { throw "Not inplemented"; }

CommandList* GraphicsVulkan::CreateCommandList() { throw "Not inplemented"; }

ConstantBuffer* GraphicsVulkan::CreateConstantBuffer(int32_t size, ConstantBufferType type) { throw "Not inplemented"; }
RenderPass* GraphicsVulkan::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	throw "Not inplemented";
}
Texture* GraphicsVulkan::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) { throw "Not inplemented"; }

Texture* GraphicsVulkan::CreateTexture(uint64_t id) { throw "Not inplemented"; }

int32_t GraphicsVulkan::GetCurrentSwapBufferIndex() const
{
	assert(currentSwapBufferIndex >= 0);
	return currentSwapBufferIndex;
}

int32_t GraphicsVulkan::GetSwapBufferCount() const { return swapBufferCount_; }

} // namespace G3
} // namespace LLGI