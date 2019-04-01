#include "LLGI.CommandListVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

CommandListVulkan::CommandListVulkan() {}

CommandListVulkan::~CommandListVulkan() { commandBuffers.clear(); }

bool CommandListVulkan::Initialize(GraphicsVulkan* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.commandPool = graphics->GetCommandPool();
	allocInfo.commandBufferCount = graphics->GetSwapBufferCount();
	commandBuffers = graphics->GetDevice().allocateCommandBuffers(allocInfo);

	return true;
}

void CommandListVulkan::Begin()
{
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];

	cmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	vk::CommandBufferBeginInfo cmdBufInfo;
	cmdBuffer.begin(cmdBufInfo);
}

void CommandListVulkan::End()
{
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];
	cmdBuffer.end();
}

void CommandListVulkan::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];

	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(x, y), vk::Extent2D(width, height));
	cmdBuffer.setScissor(0, scissor);
}

void CommandListVulkan::Draw(int32_t pritimiveCount) { throw "Not inplemented"; }

void CommandListVulkan::SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset) { throw "Not inplemented"; }

void CommandListVulkan::SetIndexBuffer(IndexBuffer* indexBuffer) { throw "Not inplemented"; }

void CommandListVulkan::SetPipelineState(PipelineState* pipelineState) { throw "Not inplemented"; }

void CommandListVulkan::SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage) { throw "Not inplemented"; }

void CommandListVulkan::SetTexture(
	Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage)
{

	throw "Not inplemented";
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

	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];

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

	vk::ClearValue clear_values[2];
	clear_values[0].color = clearColor;
	clear_values[1].depthStencil = clearDepth;

	// begin renderpass
	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.framebuffer = renderPass_->frameBuffer;
	renderPassBeginInfo.renderPass = renderPass_->renderPass;
	renderPassBeginInfo.renderArea.extent = vk::Extent2D(renderPass_->GetImageSize().X, renderPass_->GetImageSize().Y);
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clear_values;
	cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	vk::Viewport viewport = vk::Viewport(
		0.0f, 0.0f, static_cast<float>(renderPass_->GetImageSize().X), static_cast<float>(renderPass_->GetImageSize().Y), 0.0f, 1.0f);
	cmdBuffer.setViewport(0, viewport);

	vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(), vk::Extent2D(renderPass_->GetImageSize().X, renderPass_->GetImageSize().Y));
	cmdBuffer.setScissor(0, scissor);
}

void CommandListVulkan::EndRenderPass()
{
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];

	// end renderpass
	cmdBuffer.endRenderPass();
}

vk::CommandBuffer CommandListVulkan::GetCommandBuffer() const
{
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];
	return cmdBuffer;
}

} // namespace LLGI