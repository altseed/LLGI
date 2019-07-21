#include "LLGI.CommandListVulkan.h"
#include "LLGI.ConstantBufferVulkan.h"
#include "LLGI.GraphicsVulkan.h"
#include "LLGI.IndexBufferVulkan.h"
#include "LLGI.PipelineStateVulkan.h"
#include "LLGI.TextureVulkan.h"
#include "LLGI.VertexBufferVulkan.h"

namespace LLGI
{

DescriptorPoolVulkan::DescriptorPoolVulkan(GraphicsVulkan* graphics, int32_t size, int stage)
	: graphics_(graphics), size_(size), stage_(stage)
{
	std::array<vk::DescriptorPoolSize, 3> poolSizes;
	poolSizes[0].type = vk::DescriptorType::eUniformBufferDynamic;
	poolSizes[0].descriptorCount = size * stage;
	poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
	poolSizes[1].descriptorCount = size * stage;

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = size * stage;

	descriptorPool = graphics_->GetDevice().createDescriptorPool(poolInfo);
}

DescriptorPoolVulkan ::~DescriptorPoolVulkan()
{
	if (descriptorPool != nullptr)
	{
		graphics_->GetDevice().destroyDescriptorPool(descriptorPool);
		descriptorPool = nullptr;
	}
}

const std::vector<vk::DescriptorSet>& DescriptorPoolVulkan::Get(PipelineStateVulkan* pip)
{
	if (cache.size() < static_cast<size_t>(offset))
	{
		offset++;
		return cache[offset - 1];
	}

	// TODO : improve it
	vk::DescriptorSetAllocateInfo allocateInfo;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 2;
	allocateInfo.pSetLayouts = (pip->GetDescriptorSetLayout().data());

	std::vector<vk::DescriptorSet> descriptorSets = graphics_->GetDevice().allocateDescriptorSets(allocateInfo);
	cache.push_back(descriptorSets);
	offset++;
	return cache[offset - 1];
}

void DescriptorPoolVulkan::Reset() { offset = 0; }


bool VulkanNativeCommandList::Initialize(GraphicsVulkan* graphics)
{
    //vk::CommandBufferAllocateInfo allocInfo;
    //allocInfo.commandPool = graphics->GetCommandPool();
    //allocInfo.commandBufferCount = 1;
    //auto r = graphics->GetDevice().allocateCommandBuffers(allocInfo);

    graphics_ = graphics;

    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = (VkCommandPool)graphics->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    LLGI_VK_CHECK(vkAllocateCommandBuffers((VkDevice)graphics->GetDevice(), &allocInfo, &nativeCommandBuffer_));

    descriptorPool_ = std::make_shared<DescriptorPoolVulkan>(graphics, 10000, 2);

    return true;
}

void VulkanNativeCommandList::Dispose()
{
}

bool VulkanNativeCommandList::Begin()
{
    LLGI_VK_CHECK(vkResetCommandBuffer(nativeCommandBuffer_, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    LLGI_VK_CHECK(vkBeginCommandBuffer(nativeCommandBuffer_, &beginInfo));

    descriptorPool_->Reset();

    return true;
}

bool VulkanNativeCommandList::End()
{
    LLGI_VK_CHECK(vkEndCommandBuffer(nativeCommandBuffer_));
    return true;
}

CommandListVulkan::CommandListVulkan() {}

CommandListVulkan::~CommandListVulkan()
{
    for (auto& nativeList : commandLists_) {
        nativeList->Dispose();
    }
    commandLists_.clear();
}

bool CommandListVulkan::Initialize(GraphicsVulkan* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	for (size_t i = 0; i < static_cast<size_t>(graphics_->GetSwapBufferCount()); i++)
	{
        auto cl = std::make_shared<VulkanNativeCommandList>();
        if (!cl->Initialize(graphics)) {
            return false;
        }
        commandLists_.push_back(cl);
	}

	return true;
}

void CommandListVulkan::Begin()
{
	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];
    if (!cmdBuffer->Begin()) {
        assert(0);
        return;
    }

	CommandList::Begin();
}

void CommandListVulkan::End()
{
	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];
    if (!cmdBuffer->End()) {
        assert(0);
        return;
    }
}

void CommandListVulkan::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];

    VkRect2D scissor = { { x, y }, { (uint32_t)width, (uint32_t)height } };
    vkCmdSetScissor(cmdBuffer->GetNativeCommandBuffer(), 0, 1, &scissor);
}

void CommandListVulkan::Draw(int32_t pritimiveCount)
{
	BindingVertexBuffer vb_;
	IndexBuffer* ib_ = nullptr;
	PipelineState* pip_ = nullptr;

	bool isVBDirtied = false;
	bool isIBDirtied = false;
	bool isPipDirtied = false;

	GetCurrentVertexBuffer(vb_, isVBDirtied);
	GetCurrentIndexBuffer(ib_, isIBDirtied);
	GetCurrentPipelineState(pip_, isPipDirtied);

	assert(vb_.vertexBuffer != nullptr);
	assert(ib_ != nullptr);
	assert(pip_ != nullptr);

	auto vb = static_cast<VertexBufferVulkan*>(vb_.vertexBuffer);
	auto ib = static_cast<IndexBufferVulkan*>(ib_);
	auto pip = static_cast<PipelineStateVulkan*>(pip_);

	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];

	// assign a vertex buffer
	if (isVBDirtied)
	{
        VkDeviceSize vertexOffsets = vb_.offset;
        VkBuffer buffer = (VkBuffer)vb->GetBuffer();
        vkCmdBindVertexBuffers(cmdBuffer->GetNativeCommandBuffer(), 0, 1, &buffer, &vertexOffsets);
	}

	// assign an index vuffer
	if (isIBDirtied)
	{
		VkDeviceSize indexOffset = 0;
        VkIndexType indexType = VK_INDEX_TYPE_UINT16;

        if (ib->GetStride() == 2)
            indexType = VK_INDEX_TYPE_UINT16;
        if (ib->GetStride() == 4)
            indexType = VK_INDEX_TYPE_UINT32;

        VkBuffer buffer = (VkBuffer)ib->GetBuffer();
        vkCmdBindIndexBuffer(cmdBuffer->GetNativeCommandBuffer(), buffer, indexOffset, indexType);
	}

    auto* dp = cmdBuffer->GetDescriptorPool();

	std::vector<vk::DescriptorSet> descriptorSets = dp->Get(pip);
	/*
	vk::DescriptorSetAllocateInfo allocateInfo;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 2;
	allocateInfo.pSetLayouts = (pip->GetDescriptorSetLayout().data());

	std::vector<vk::DescriptorSet> descriptorSets = graphics_->GetDevice().allocateDescriptorSets(allocateInfo);
	*/

	std::array<vk::WriteDescriptorSet, 16> writeDescriptorSets;
	int writeDescriptorIndex = 0;

	std::array<vk::DescriptorBufferInfo, 16> descriptorBufferInfos;
	int descriptorBufferIndex = 0;

	std::array<vk::DescriptorImageInfo, 16> descriptorImageInfos;
	int descriptorImageIndex = 0;

	std::array<bool, static_cast<int>(ShaderStageType::Max)> stages;
	stages.fill(false);

	ConstantBuffer* vcb = nullptr;
	GetCurrentConstantBuffer(ShaderStageType::Vertex, vcb);
	if (vcb != nullptr)
	{
		stages[0] = true;

		descriptorBufferInfos[descriptorBufferIndex].buffer = (static_cast<ConstantBufferVulkan*>(vcb)->GetBuffer());
		descriptorBufferInfos[descriptorBufferIndex].offset = 0;
		descriptorBufferInfos[descriptorBufferIndex].range = vcb->GetSize();

		vk::WriteDescriptorSet desc;
		desc.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		desc.dstSet = descriptorSets[0];
		desc.dstBinding = 0;
		desc.dstArrayElement = 0;
		desc.pBufferInfo = &(descriptorBufferInfos[descriptorBufferIndex]);
		desc.descriptorCount = 1;

		writeDescriptorSets[writeDescriptorIndex] = desc;

		descriptorBufferIndex++;
		writeDescriptorIndex++;
	}

	ConstantBuffer* pcb = nullptr;
	GetCurrentConstantBuffer(ShaderStageType::Pixel, pcb);
	if (pcb != nullptr)
	{
		stages[1] = true;

		descriptorBufferInfos[descriptorBufferIndex].buffer = (static_cast<ConstantBufferVulkan*>(pcb)->GetBuffer());
		descriptorBufferInfos[descriptorBufferIndex].offset = 0;
		descriptorBufferInfos[descriptorBufferIndex].range = pcb->GetSize();
		vk::WriteDescriptorSet desc;
		desc.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		desc.dstSet = descriptorSets[1];
		desc.dstBinding = 0;
		desc.dstArrayElement = 0;
		desc.pBufferInfo = &(descriptorBufferInfos[descriptorBufferIndex]);
		desc.descriptorCount = 1;

		writeDescriptorSets[writeDescriptorIndex] = desc;

		descriptorBufferIndex++;
		writeDescriptorIndex++;
	}

	// Assign textures
	for (int stage_ind = 0; stage_ind < (int32_t)ShaderStageType::Max; stage_ind++)
	{
		for (int unit_ind = 0; unit_ind < currentTextures[stage_ind].size(); unit_ind++)
		{
			if (currentTextures[stage_ind][unit_ind].texture == nullptr)
				continue;

			stages[stage_ind] = true;

			auto texture = (TextureVulkan*)currentTextures[stage_ind][unit_ind].texture;
			auto wm = (int32_t)currentTextures[stage_ind][unit_ind].wrapMode;
			auto mm = (int32_t)currentTextures[stage_ind][unit_ind].minMagFilter;

			vk::DescriptorImageInfo imageInfo;
			imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageInfo.imageView = texture->GetView();
			imageInfo.sampler = graphics_->GetDefaultSampler();
			descriptorImageInfos[descriptorImageIndex] = imageInfo;

			vk::WriteDescriptorSet desc;
			desc.dstSet = descriptorSets[stage_ind];
			desc.dstBinding = unit_ind + 1;
			desc.dstArrayElement = 0;
			desc.pImageInfo = &descriptorImageInfos[descriptorImageIndex];
			desc.descriptorCount = 1;
			desc.descriptorType = vk::DescriptorType::eCombinedImageSampler;

			writeDescriptorSets[writeDescriptorIndex] = desc;

			descriptorImageIndex++;
			writeDescriptorIndex++;
		}
	}

	graphics_->GetDevice().updateDescriptorSets(writeDescriptorIndex, writeDescriptorSets.data(), 0, nullptr);

	std::array<VkDescriptorSet, static_cast<int>(ShaderStageType::Max)> descriptorSets_;
	std::array<uint32_t, static_cast<int>(ShaderStageType::Max)> offsets_;
	int descriptorIndex = 0;
	int firstSet = -1;

	for (int i = 0; i < static_cast<int>(ShaderStageType::Max); i++)
	{
		if (!stages[i])
			continue;

		descriptorSets_[descriptorIndex] = static_cast<VkDescriptorSet>(descriptorSets[i]);
		offsets_[descriptorIndex] = 0;

		if (firstSet < 0)
		{
			firstSet = descriptorIndex;
		}

		descriptorIndex++;
	}

	if (firstSet >= 0)
	{
        auto pipelineLayout = (VkPipelineLayout)pip->GetPipelineLayout();
        vkCmdBindDescriptorSets(
            cmdBuffer->GetNativeCommandBuffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            firstSet,
            descriptorIndex,
            descriptorSets_.data(),
            descriptorIndex,
            offsets_.data());
	}

	// assign a pipeline
	if (isPipDirtied)
	{
        auto pipeline = (VkPipeline)pip->GetPipeline();
        vkCmdBindPipeline(cmdBuffer->GetNativeCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	// draw
	int indexPerPrim = 0;
	if (pip->Topology == TopologyType::Triangle)
		indexPerPrim = 3;
	if (pip->Topology == TopologyType::Line)
		indexPerPrim = 2;

    vkCmdDrawIndexed(cmdBuffer->GetNativeCommandBuffer(), indexPerPrim * pritimiveCount, 1, 0, 0, 0);

	CommandList::Draw(pritimiveCount);
}

void CommandListVulkan::BeginRenderPass(RenderPass* renderPass)
{
	auto renderPass_ = static_cast<RenderPassVulkan*>(renderPass);

	vk::ClearColorValue clearColor(std::array<float, 4>{renderPass_->GetClearColor().R / 255.0f,
														renderPass_->GetClearColor().G / 255.0f,
														renderPass_->GetClearColor().B / 255.0f,
														renderPass_->GetClearColor().A / 255.0f});
	vk::ClearDepthStencilValue clearDepth(1.0f, 0);

	vk::ImageSubresourceRange colorSubRange;
	colorSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	colorSubRange.levelCount = 1;
	colorSubRange.layerCount = 1;

	vk::ImageSubresourceRange depthSubRange;
	depthSubRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	depthSubRange.levelCount = 1;
	depthSubRange.layerCount = 1;

	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];

	/*
	// to make screen clear
	SetImageLayout(
		cmdBuffer, renderPass_->colorBuffers[0], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, colorSubRange);
	SetImageLayout(cmdBuffer,
				   renderPass_->depthBuffer,
				   vk::ImageLayout::eDepthStencilAttachmentOptimal,
				   vk::ImageLayout::eTransferDstOptimal,
				   depthSubRange);

	if (renderPass->GetIsColorCleared())
	{
		cmdBuffer.clearColorImage(renderPass_->colorBuffers[0], vk::ImageLayout::eTransferDstOptimal, clearColor, colorSubRange);
	}

	if (renderPass->GetIsDepthCleared())
	{
		cmdBuffer.clearDepthStencilImage(renderPass_->depthBuffer, vk::ImageLayout::eTransferDstOptimal, clearDepth, depthSubRange);
	}

	// to draw
	SetImageLayout(cmdBuffer,
				   renderPass_->colorBuffers[0],
				   vk::ImageLayout::eTransferDstOptimal,
				   vk::ImageLayout::eColorAttachmentOptimal,
				   colorSubRange);

	SetImageLayout(cmdBuffer,
				   renderPass_->depthBuffer,
				   vk::ImageLayout::eTransferDstOptimal,
				   vk::ImageLayout::eDepthStencilAttachmentOptimal,
				   depthSubRange);
	*/

	VkClearValue clear_values[2];
	clear_values[0].color = clearColor;
	clear_values[1].depthStencil = clearDepth;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = static_cast<VkRenderPass>(renderPass_->renderPassPipelineState->GetRenderPass());
    renderPassBeginInfo.framebuffer = static_cast<VkFramebuffer>(renderPass_->frameBuffer);
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent.width = renderPass_->GetImageSize().X;
    renderPassBeginInfo.renderArea.extent.height = renderPass_->GetImageSize().Y;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clear_values;
    vkCmdBeginRenderPass(cmdBuffer->GetNativeCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(renderPass_->GetImageSize().X), static_cast<float>(renderPass_->GetImageSize().Y), 0.0f, 1.0f };
    vkCmdSetViewport(cmdBuffer->GetNativeCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor = { { 0, 0 }, { (uint32_t)renderPass_->GetImageSize().X, (uint32_t)renderPass_->GetImageSize().Y } };
    vkCmdSetScissor(cmdBuffer->GetNativeCommandBuffer(), 0, 1, &scissor);
}

void CommandListVulkan::EndRenderPass()
{
	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];

    // end renderpass
    vkCmdEndRenderPass(cmdBuffer->GetNativeCommandBuffer());
}

VkCommandBuffer CommandListVulkan::GetCommandBuffer() const
{
	auto& cmdBuffer = commandLists_[graphics_->GetCurrentSwapBufferIndex()];
    return cmdBuffer->GetNativeCommandBuffer();
}

} // namespace LLGI