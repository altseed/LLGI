
#include "LLGI.CommandList.h"
#include "LLGI.ConstantBuffer.h"
#include "LLGI.IndexBuffer.h"
#include "LLGI.PipelineState.h"
#include "LLGI.Texture.h"
#include "LLGI.VertexBuffer.h"

namespace LLGI
{

void CommandList::GetCurrentVertexBuffer(BindingVertexBuffer& buffer, bool& isDirtied)
{

	buffer = bindingVertexBuffer;
	isDirtied = isVertexBufferDirtied;
}

void CommandList::GetCurrentIndexBuffer(IndexBuffer*& buffer, bool& isDirtied)
{
	buffer = currentIndexBuffer;
	isDirtied = isCurrentIndexBufferDirtied;
}

void CommandList::GetCurrentPipelineState(PipelineState*& pipelineState, bool& isDirtied)
{
	pipelineState = currentPipelineState;
	isDirtied = isPipelineDirtied;
}

void CommandList::GetCurrentConstantBuffer(ShaderStageType type, ConstantBuffer*& buffer)
{
	buffer = constantBuffers[static_cast<int>(type)];
}

CommandList::CommandList()
{
	constantBuffers.fill(nullptr);

	for (auto& t : currentTextures)
	{
		for (auto& bt : t)
		{
			bt.texture = nullptr;
		}
	}
}

CommandList::~CommandList()
{
	for (auto& c : constantBuffers)
	{
		SafeRelease(c);
	}

	for (auto& t : currentTextures)
	{
		for (auto& bt : t)
		{
			SafeRelease(bt.texture);
		}
	}
}

void CommandList::Begin()
{
	bindingVertexBuffer.vertexBuffer = nullptr;
	currentIndexBuffer = nullptr;
	currentPipelineState = nullptr;
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
}

void CommandList::End() {}

void CommandList::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) {}

void CommandList::Draw(int32_t pritimiveCount)
{
	isVertexBufferDirtied = false;
	isCurrentIndexBufferDirtied = false;
	isPipelineDirtied = false;
}

void CommandList::SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset)
{
	isVertexBufferDirtied =
		bindingVertexBuffer.vertexBuffer != vertexBuffer || bindingVertexBuffer.stride != stride || bindingVertexBuffer.offset != offset;
	bindingVertexBuffer.vertexBuffer = vertexBuffer;
	bindingVertexBuffer.stride = stride;
	bindingVertexBuffer.offset = offset;
}

void CommandList::SetIndexBuffer(IndexBuffer* indexBuffer)
{
	isCurrentIndexBufferDirtied = currentIndexBuffer != indexBuffer;
	currentIndexBuffer = indexBuffer;
}

void CommandList::SetPipelineState(PipelineState* pipelineState)
{
	currentPipelineState = pipelineState;
	isPipelineDirtied = true;
}

void CommandList::SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage)
{
	auto ind = static_cast<int>(shaderStage);
	SafeAssign(constantBuffers[ind], constantBuffer);
}

void CommandList::SetTexture(
	Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage)
{
	auto ind = static_cast<int>(shaderStage);
	SafeAssign(currentTextures[ind][unit].texture, texture);
	currentTextures[ind][unit].wrapMode = wrapMode;
	currentTextures[ind][unit].minMagFilter = minmagFilter;
}

} // namespace LLGI
