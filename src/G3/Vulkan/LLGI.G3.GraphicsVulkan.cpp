#include "LLGI.G3.GraphicsVulkan.h"

namespace LLGI
{
namespace G3
{

GraphicsVulkan::GraphicsVulkan(const vk::Device& device,
							   const vk::CommandPool& commandPool,
							   int32_t swapBufferCount,
							   std::function<void(PlatformStatus&)> getStatus)
	: vkDevice(device), vkCmdPool(commandPool), swapBufferCount_(swapBufferCount), getStatus_(getStatus)
{
	currentRenderPass = std::make_shared<RenderPassVulkan>();
}

GraphicsVulkan::~GraphicsVulkan() {}

void GraphicsVulkan::NewFrame()
{
	currentSwapBufferIndex = (currentSwapBufferIndex + 1) % swapBufferCount_;

	PlatformStatus status;
	getStatus_(status);

	assert(currentSwapBufferIndex != status.currentSwapBufferIndex);

	currentColorBuffer = status.colorBuffer;
}

void GraphicsVulkan::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsVulkan::Execute(CommandList* commandList) { throw "Not inplemented"; }

void GraphicsVulkan::WaitFinish() { throw "Not inplemented"; }

RenderPass* GraphicsVulkan::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	currentRenderPass->SetClearColor(clearColor);
	currentRenderPass->SetIsColorCleared(isColorCleared);
	// TODO : check specification
	currentRenderPass->SetIsDepthCleared(false);
	currentRenderPass->colorBuffers[0] = currentColorBuffer;
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