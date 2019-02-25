#include "LLGI.G3.GraphicsVulkan.h"

namespace LLGI
{
namespace G3
{

GraphicsVulkan::GraphicsVulkan(int32_t swapBufferCount) : swapBufferCount_(swapBufferCount) {}

GraphicsVulkan::~GraphicsVulkan() {}

void GraphicsVulkan::NewFrame() { throw "Not inplemented"; }

void GraphicsVulkan::SetWindowSize(const Vec2I& windowSize) { throw "Not inplemented"; }

void GraphicsVulkan::Execute(CommandList* commandList) { throw "Not inplemented"; }

void GraphicsVulkan::WaitFinish() { throw "Not inplemented"; }

RenderPass* GraphicsVulkan::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	throw "Not inplemented";
}

VertexBuffer* GraphicsVulkan::CreateVertexBuffer(int32_t size) { throw "Not inplemented"; }

IndexBuffer* GraphicsVulkan::CreateIndexBuffer(int32_t stride, int32_t count) { throw "Not inplemented"; }

Shader* GraphicsVulkan::CreateShader(DataStructure* data, int32_t count) { throw "Not inplemented"; }

PipelineState* GraphicsVulkan::CreatePiplineState() { throw "Not inplemented"; }

CommandList* GraphicsVulkan::CreateCommandList() { throw "Not inplemented"; }

ConstantBuffer* GraphicsVulkan::CreateConstantBuffer(int32_t size, ConstantBufferType type = ConstantBufferType::LongTime)
{
	throw "Not inplemented";
}
RenderPass* GraphicsVulkan::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	throw "Not inplemented";
}
Texture* GraphicsVulkan::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) { throw "Not inplemented"; }

Texture* GraphicsVulkan::CreateTexture(uint64_t id) { throw "Not inplemented"; }

} // namespace G3
} // namespace LLGI