#include "LLGI.G3.CommandListVulkan.h"
#include "LLGI.G3.GraphicsVulkan.h"

namespace LLGI
{
namespace G3
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

void CommandListVulkan::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) { throw "Not inplemented"; }

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
	throw "Not inplemented";

	auto renderPass_ = static_cast<RenderPassVulkan*>(renderPass);

	vk::ClearColorValue clearColor(std::array<float, 4>{0.0f, 0.0f, 0.5f, 1.0f});
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

	if (renderPass->GetIsColorCleared())
	{
		cmdBuffer.clearColorImage(renderPass_->colorBuffers[0], vk::ImageLayout::eTransferDstOptimal, clearColor, colorSubRange);
	}

	if (renderPass->GetIsDepthCleared())
	{
		cmdBuffer.clearDepthStencilImage(renderPass_->depthBuffer, vk::ImageLayout::eTransferDstOptimal, clearDepth, depthSubRange);
	}
}

void CommandListVulkan::EndRenderPass() { 
	throw "Not inplemented"; 
	auto& cmdBuffer = commandBuffers[graphics_->GetCurrentSwapBufferIndex()];
	cmdBuffer.endRenderPass();
}

} // namespace G3
} // namespace LLGI