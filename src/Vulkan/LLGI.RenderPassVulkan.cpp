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

bool RenderPassVulkan::Initialize(const TextureVulkan** textures, int32_t textureCount, TextureVulkan* depthTexture)
{
	if (textureCount == 0)
		return false;

	if (!assignRenderTextures((Texture**)(textures), textureCount))
	{
		return false;
	}

	if (!assignDepthTexture(depthTexture))
	{
		return false;
	}

	if (!getSize(screenSize_, reinterpret_cast<const Texture**>(textures), textureCount, depthTexture))
	{
		return false;
	}

	renderTargetProperties.resize(textureCount);

	for (int32_t i = 0; i < textureCount; i++)
	{
		auto texture = const_cast<TextureVulkan*>(textures[i]);
		SafeAddRef(texture);
		renderTargetProperties.at(i).colorBufferPtr = CreateSharedPtr(texture);
	}

	for (size_t i = 0; i < textureCount; i++)
	{
		renderTargetProperties.at(i).format = textures[i]->GetVulkanFormat();
	}

	FixedSizeVector<vk::ImageView, RenderTargetMax + 1> views;
	FixedSizeVector<vk::Format, RenderTargetMax> formats;
	views.resize(textureCount + 1);
	formats.resize(textureCount);

	for (int32_t i = 0; i < textureCount; i++)
	{
		views.at(i) = textures[i]->GetView();
		formats.at(i) = textures[i]->GetVulkanFormat();
	}

	if (GetHasDepthTexture())
	{
		views.at(textureCount) = depthTexture->GetView();
	}

	this->renderPassPipelineState = renderPassPipelineStateCache_->Create(GetIsSwapchainScreen(), GetHasDepthTexture(), formats);

	vk::FramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.renderPass = renderPassPipelineState->GetRenderPass();
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(views.size());
	framebufferCreateInfo.pAttachments = views.data();
	framebufferCreateInfo.width = screenSize_.X;
	framebufferCreateInfo.height = screenSize_.Y;
	framebufferCreateInfo.layers = 1;

	frameBuffer_ = device_.createFramebuffer(framebufferCreateInfo);

	return true;
}

Vec2I RenderPassVulkan::GetImageSize() const { return screenSize_; }

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