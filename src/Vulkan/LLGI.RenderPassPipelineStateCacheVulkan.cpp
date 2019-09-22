#include "LLGI.RenderPassPipelineStateCacheVulkan.h"

namespace LLGI
{

RenderPassPipelineStateCacheVulkan::RenderPassPipelineStateCacheVulkan(vk::Device device, ReferenceObject* owner)
	: device_(device), owner_(owner)
{
	SafeAddRef(owner_);
}

RenderPassPipelineStateCacheVulkan::~RenderPassPipelineStateCacheVulkan()
{
	renderPassPipelineStates_.clear();
	SafeRelease(owner_);
}

RenderPassPipelineStateVulkan*
RenderPassPipelineStateCacheVulkan::Create(bool isPresentMode, bool hasDepth, const FixedSizeVector<vk::Format, 4>& formats)
{
	RenderPassPipelineStateVulkanKey key;
	key.isPresentMode = isPresentMode;
	key.hasDepth = hasDepth;
	key.formats = formats;

	// already?
	{
		auto it = renderPassPipelineStates_.find(key);

		if (it != renderPassPipelineStates_.end())
		{
			auto ret = it->second;

			if (ret != nullptr)
			{
				auto retptr = ret.get();
				SafeAddRef(retptr);
				return retptr;
			}
		}
	}

	// settings
	std::array<vk::AttachmentDescription, 2> attachmentDescs;
	std::array<vk::AttachmentReference, 2> attachmentRefs;

	// color buffer
	attachmentDescs[0].format = formats.at(0);
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

		auto renderPass = device_.createRenderPass(renderPassInfo);
		if (!renderPass)
		{
			return nullptr;
		}

		std::shared_ptr<RenderPassPipelineStateVulkan> ret = CreateSharedPtr(new RenderPassPipelineStateVulkan(device_, owner_));
		ret->renderPass_ = renderPass;

		renderPassPipelineStates_[key] = ret;

		auto retptr = ret.get();
		SafeAddRef(retptr);
		return retptr;
	}
}

} // namespace LLGI