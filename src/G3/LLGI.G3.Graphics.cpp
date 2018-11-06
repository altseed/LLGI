#include "LLGI.G3.Graphics.h"

namespace LLGI
{
namespace G3
{

void RenderPass::SetIsColorCleared(bool isColorCleared)
{
	isColorCleared_ = isColorCleared;
}

void RenderPass::SetIsDepthCleared(bool isDepthCleared)
{
	isDepthCleared_ = isDepthCleared;
}

void RenderPass::SetClearColor(const Color8& color)
{
	color_ = color;
}

void Graphics::NewFrame()
{

}

void Graphics::SetWindowSize(const Vec2I& windowSize)
{
	windowSize_ = windowSize;
}

void Graphics::Execute(CommandList* commandList)
{

}

RenderPass* Graphics::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	return nullptr;
}

VertexBuffer* Graphics::CreateVertexBuffer(int32_t size)
{
	return nullptr;
}

IndexBuffer* Graphics::CreateIndexBuffer(int32_t stride, int32_t count)
{
	return nullptr;
}

Shader* Graphics::CreateShader(DataStructure* data, int32_t count)
{
	return nullptr;
}

PipelineState* Graphics::CreatePiplineState()
{
	return nullptr;
}

CommandList* Graphics::CreateCommandList()
{
	return nullptr;
}

ConstantBuffer* Graphics::CreateConstantBuffer(int32_t size, ConstantBufferType type)
{
	return nullptr;
}

Texture* Graphics::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	return nullptr;
}

Texture* Graphics::CreateTexture(uint64_t id)
{
	return nullptr;
}

}
}