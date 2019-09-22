#include "LLGI.RenderPassVulkan.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.CommandListVulkan.h"
#include "LLGI.ConstantBufferVulkan.h"
#include "LLGI.GraphicsVulkan.h"
#include "LLGI.IndexBufferVulkan.h"
#include "LLGI.PipelineStateVulkan.h"
#include "LLGI.ShaderVulkan.h"
#include "LLGI.SingleFrameMemoryPoolVulkan.h"
#include "LLGI.TextureVulkan.h"
#include "LLGI.VertexBufferVulkan.h"

namespace LLGI
{

RenderPassVulkan::RenderPassVulkan(RenderPassPipelineStateCacheVulkan* renderPassPipelineStateCache,
								   vk::Device device,
								   ReferenceObject* owner)
	: renderPassPipelineStateCache_(renderPassPipelineStateCache), device_(device), owner_(owner)
{
	SafeAddRef(renderPassPipelineStateCache_);
	SafeAddRef(owner_);
}

RenderPassVulkan::~RenderPassVulkan()
{
	if (frameBuffer_)
	{
		device_.destroyFramebuffer(frameBuffer_);
	}

	SafeRelease(renderPassPipelineState);
	SafeRelease(renderPassPipelineStateCache_);
	SafeRelease(owner_);
}

bool RenderPassVulkan::Initialize(const vk::Image& imageColor,
								  const vk::Image& imageDepth,
								  const vk::ImageView& imageColorView,
								  const vk::ImageView& imageDepthView,
								  Vec2I imageSize,
								  vk::Format format)
{
	imageSize_ = imageSize;

	hasDepth = true;
	isPresentMode = true;
	renderTargetProperties.resize(1);

	FixedSizeVector<vk::Format, 4> formats;
	formats.resize(1);
	formats.at(0) = format;
	this->renderPassPipelineState = renderPassPipelineStateCache_->Create(isPresentMode, hasDepth, formats);

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

	frameBuffer_ = device_.createFramebuffer(framebufferCreateInfo);

	renderTargetProperties.at(0).colorBuffer = imageColor;
	depthBuffer = imageDepth;
	/*
	auto texture = CreateSharedPtr(new TextureVulkan(graphics_, false));
	if (!texture->InitializeAsScreen(imageColor, imageColorView, format, imageSize))
	{
		return false;
	}
	renderTargetProperties.at(0).colorBufferPtr = texture;
	renderTargetProperties.at(0).format = format;
	*/
	return true;
}

bool RenderPassVulkan::Initialize(const TextureVulkan** textures, int32_t textureCount, TextureVulkan* depthTexture)
{
	if (textureCount == 0)
		return false;

	renderTargetProperties.resize(textureCount);

	for (int32_t i = 0; i < textureCount; i++)
	{
		auto texture = const_cast<TextureVulkan*>(textures[i]);
		SafeAddRef(texture);
		renderTargetProperties.at(i).colorBufferPtr = CreateSharedPtr(texture);
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

	for (size_t i = 0; i < textureCount; i++)
	{
		renderTargetProperties.at(i).format = textures[i]->GetVulkanFormat();
	}

	// TODO : make const
	imageSize_ = ((TextureVulkan*)textures[0])->GetSizeAs2D();

	hasDepth = depthTexture != nullptr;
	isPresentMode = true;

	assert(depthTexture != nullptr);

	std::array<vk::ImageView, 2> views;
	views[0] = textures[0]->GetView();
	views[1] = depthTexture->GetView();

	FixedSizeVector<vk::Format, 4> formats;
	formats.resize(1);
	formats.at(0) = textures[0]->GetVulkanFormat();
	this->renderPassPipelineState = renderPassPipelineStateCache_->Create(isPresentMode, hasDepth, formats);

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPassPipelineState->GetRenderPass();
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(views.size());
	framebufferCreateInfo.pAttachments = views.data();
	framebufferCreateInfo.width = imageSize_.X;
	framebufferCreateInfo.height = imageSize_.Y;
	framebufferCreateInfo.layers = 1;

	frameBuffer_ = device_.createFramebuffer(framebufferCreateInfo);

	return true;
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

Texture* RenderPassVulkan::GetColorBuffer(int index) { return renderTargetProperties.at(index).colorBufferPtr.get(); }

RenderPassPipelineState* RenderPassVulkan::CreateRenderPassPipelineState()
{
	auto ret = renderPassPipelineState;
	SafeAddRef(ret);
	return ret;
}

RenderPassPipelineStateVulkan::RenderPassPipelineStateVulkan(vk::Device device, ReferenceObject* owner)
{
	device_ = device;
	owner_ = owner;
	SafeAddRef(owner);
}

RenderPassPipelineStateVulkan::~RenderPassPipelineStateVulkan()
{
	if (renderPass_)
	{
		device_.destroyRenderPass(renderPass_);
	}

	SafeRelease(owner_);
}

vk::RenderPass RenderPassPipelineStateVulkan::GetRenderPass() const { return renderPass_; }

} // namespace LLGI