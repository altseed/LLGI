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

RenderPassPipelineStateVulkan* RenderPassPipelineStateCacheVulkan::Create(const RenderPassPipelineStateKey key)
{
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
	bool hasDepth = key.DepthFormat != TextureFormatType::Unknown;
	FixedSizeVector<vk::AttachmentDescription, RenderTargetMax + 1> attachmentDescs;
	FixedSizeVector<vk::AttachmentReference, RenderTargetMax + 1> attachmentRefs;
	FixedSizeVector<vk::ImageLayout, RenderTargetMax + 1> finalLayouts;
	attachmentDescs.resize(key.RenderTargetFormats.size() + (hasDepth ? 1 : 0));
	attachmentRefs.resize(key.RenderTargetFormats.size() + (hasDepth ? 1 : 0));
	finalLayouts.resize(key.RenderTargetFormats.size() + (hasDepth ? 1 : 0));

	// color buffer
	int colorCount = static_cast<int32_t>(key.RenderTargetFormats.size());

	for (int i = 0; i < colorCount; i++)
	{
		attachmentDescs.at(i).format = (vk::Format)VulkanHelper::TextureFormatToVkFormat(key.RenderTargetFormats.at(i));
		attachmentDescs.at(i).samples = vk::SampleCountFlagBits::e1;

		if (key.IsColorCleared)
			attachmentDescs.at(i).loadOp = vk::AttachmentLoadOp::eClear;
		else
			attachmentDescs.at(i).loadOp = vk::AttachmentLoadOp::eDontCare;

		attachmentDescs.at(i).storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescs.at(i).stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescs.at(i).stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	}

	if (key.IsPresent)
	{
		// When clearing, the initialLayout does not matter.
		attachmentDescs.at(0).initialLayout = (key.IsColorCleared) ? vk::ImageLayout::eUndefined : vk::ImageLayout::ePresentSrcKHR;
		attachmentDescs.at(0).finalLayout = vk::ImageLayout::ePresentSrcKHR;
	}
	else
	{
		for (int i = 0; i < colorCount; i++)
		{
			// When clearing, the initialLayout does not matter.
			attachmentDescs.at(i).initialLayout =
				(key.IsColorCleared) ? vk::ImageLayout::eUndefined : vk::ImageLayout::eShaderReadOnlyOptimal;
			attachmentDescs.at(i).finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
	}

	// depth buffer
	if (hasDepth)
	{
		attachmentDescs.at(colorCount).format = (vk::Format)VulkanHelper::TextureFormatToVkFormat(key.DepthFormat);
		attachmentDescs.at(colorCount).samples = vk::SampleCountFlagBits::e1;

		if (key.IsDepthCleared)
			attachmentDescs.at(colorCount).loadOp = vk::AttachmentLoadOp::eClear;
		else
			attachmentDescs.at(colorCount).loadOp = vk::AttachmentLoadOp::eDontCare;

		attachmentDescs.at(colorCount).storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescs.at(colorCount).stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescs.at(colorCount).stencilStoreOp = vk::AttachmentStoreOp::eStore;

		// When clearing, the initialLayout does not matter.
		attachmentDescs.at(colorCount).initialLayout =
			(key.IsDepthCleared) ? vk::ImageLayout::eUndefined : vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachmentDescs.at(colorCount).finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	for (size_t i = 0; i < attachmentDescs.size(); i++)
	{
		finalLayouts.at(i) = attachmentDescs.at(i).finalLayout;
	}

	for (int i = 0; i < colorCount; i++)
	{
		vk::AttachmentReference& colorReference = attachmentRefs.at(i);
		colorReference.attachment = i;
		colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	if (hasDepth)
	{
		vk::AttachmentReference& depthReference = attachmentRefs.at(colorCount);
		depthReference.attachment = colorCount;
		depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	std::array<vk::SubpassDescription, 1> subpasses;
	{
		vk::SubpassDescription& subpass = subpasses[0];
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = colorCount;
		subpass.pColorAttachments = &attachmentRefs.at(0);

		if (hasDepth)
		{
			subpass.pDepthStencilAttachment = &attachmentRefs.at(colorCount);
		}
		else
		{
			subpass.pDepthStencilAttachment = nullptr;
		}
	}

	std::array<vk::SubpassDependency, RenderTargetMax * 2> dependencies;

	if (!key.IsPresent)
	{
		for (int i = 0; i < colorCount; i++)
		{
			dependencies[i * 2 + 0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[i * 2 + 0].dstSubpass = i;
			dependencies[i * 2 + 0].srcStageMask = (vk::PipelineStageFlags)VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[i * 2 + 0].dstStageMask = (vk::PipelineStageFlags)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[i * 2 + 0].srcAccessMask = (vk::AccessFlags)VK_ACCESS_SHADER_READ_BIT;
			dependencies[i * 2 + 0].dstAccessMask = (vk::AccessFlags)VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[i * 2 + 0].dependencyFlags = (vk::DependencyFlags)VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[i * 2 + 1].srcSubpass = i;
			dependencies[i * 2 + 1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[i * 2 + 1].srcStageMask = (vk::PipelineStageFlags)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[i * 2 + 1].dstStageMask = (vk::PipelineStageFlags)VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[i * 2 + 1].srcAccessMask = (vk::AccessFlags)VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[i * 2 + 1].dstAccessMask = (vk::AccessFlags)VK_ACCESS_SHADER_READ_BIT;
			dependencies[i * 2 + 1].dependencyFlags = (vk::DependencyFlags)VK_DEPENDENCY_BY_REGION_BIT;
		}
	}

	{
		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescs.size();
		renderPassInfo.pAttachments = attachmentDescs.data();

		renderPassInfo.subpassCount = (uint32_t)subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();

		if (!key.IsPresent)
		{
			renderPassInfo.dependencyCount = static_cast<uint32_t>(colorCount * 2);
			renderPassInfo.pDependencies = dependencies.data();
		}
		else
		{
			renderPassInfo.dependencyCount = 0;
			renderPassInfo.pDependencies = nullptr;
		}

		auto renderPass = device_.createRenderPass(renderPassInfo);
		if (!renderPass)
		{
			return nullptr;
		}

		std::shared_ptr<RenderPassPipelineStateVulkan> ret = CreateSharedPtr(new RenderPassPipelineStateVulkan(device_, owner_));
		ret->renderPass_ = renderPass;
		ret->finalLayouts_ = finalLayouts;
		renderPassPipelineStates_[key] = ret;

		auto retptr = ret.get();
		SafeAddRef(retptr);

		retptr->RenderTargetCount = colorCount;

		return retptr;
	}
}

} // namespace LLGI
