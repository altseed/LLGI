
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

CommandList::CommandList(int32_t swapCount) : swapCount_(swapCount)
{
	constantBuffers.fill(nullptr);

	for (auto& t : currentTextures)
	{
		for (auto& bt : t)
		{
			bt.texture = nullptr;
		}
	}

	swapObjects.resize(swapCount_);
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

	for (auto& so : swapObjects)
	{
		for (auto& o : so.referencedObjects)
		{
			o->Release();
		}
		so.referencedObjects.clear();
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

	swapIndex_ = (swapIndex_ + 1) % swapCount_;

	for (auto& o : swapObjects[swapIndex_].referencedObjects)
	{
		o->Release();
	}
	swapObjects[swapIndex_].referencedObjects.clear();
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
	isVertexBufferDirtied |=
		bindingVertexBuffer.vertexBuffer != vertexBuffer || bindingVertexBuffer.stride != stride || bindingVertexBuffer.offset != offset;
	bindingVertexBuffer.vertexBuffer = vertexBuffer;
	bindingVertexBuffer.stride = stride;
	bindingVertexBuffer.offset = offset;

	// register it to referenced objects
	assert(swapIndex_ >= 0);
	SafeAddRef(vertexBuffer);
	swapObjects[swapIndex_].referencedObjects.push_back(vertexBuffer);
}

void CommandList::SetIndexBuffer(IndexBuffer* indexBuffer)
{
	isCurrentIndexBufferDirtied |= currentIndexBuffer != indexBuffer;
	currentIndexBuffer = indexBuffer;

	// register it to referenced objects
	assert(swapIndex_ >= 0);
	SafeAddRef(indexBuffer);
	swapObjects[swapIndex_].referencedObjects.push_back(indexBuffer);
}

void CommandList::SetPipelineState(PipelineState* pipelineState)
{
	currentPipelineState = pipelineState;
	isPipelineDirtied = true;

	// register it to referenced objects
	assert(swapIndex_ >= 0);
	SafeAddRef(pipelineState);
	swapObjects[swapIndex_].referencedObjects.push_back(pipelineState);
}

void CommandList::SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage)
{
	auto ind = static_cast<int>(shaderStage);
	SafeAssign(constantBuffers[ind], constantBuffer);

	// register it to referenced objects
	assert(swapIndex_ >= 0);
	SafeAddRef(constantBuffer);
	swapObjects[swapIndex_].referencedObjects.push_back(constantBuffer);
}

void CommandList::SetTexture(
	Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage)
{
	auto ind = static_cast<int>(shaderStage);
	SafeAssign(currentTextures[ind][unit].texture, texture);
	currentTextures[ind][unit].wrapMode = wrapMode;
	currentTextures[ind][unit].minMagFilter = minmagFilter;

	// register it to referenced objects
	assert(swapIndex_ >= 0);
	SafeAddRef(texture);
	swapObjects[swapIndex_].referencedObjects.push_back(texture);
}

void CommandList::BeginRenderPass(RenderPass* renderPass)
{
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
}

} // namespace LLGI
