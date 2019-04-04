
#include "LLGI.CommandList.h"

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

void CommandList::GetCurrentPipelineState(PipelineState*& pipelineState) { pipelineState = currentPipelineState; }

void CommandList::Begin()
{
	bindingVertexBuffer.vertexBuffer = nullptr;
	currentIndexBuffer = nullptr;
	currentPipelineState = nullptr;
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
}

void CommandList::End() {}

void CommandList::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) {}

void CommandList::Draw(int32_t pritimiveCount)
{
	isVertexBufferDirtied = false;
	isCurrentIndexBufferDirtied = false;
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

void CommandList::SetPipelineState(PipelineState* pipelineState) { currentPipelineState = pipelineState; }

void CommandList::SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage) {}

void CommandList::SetTexture(
	Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage)
{
}

} // namespace LLGI