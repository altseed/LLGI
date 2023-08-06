
#include "LLGI.CommandList.h"
#include "LLGI.Buffer.h"
#include "LLGI.PipelineState.h"
#include "LLGI.Texture.h"

namespace LLGI
{

void CommandList::GetCurrentVertexBuffer(BindingVertexBuffer& buffer, bool& isDirtied)
{
	buffer = bindingVertexBuffer;
	isDirtied = isVertexBufferDirtied;
}

void CommandList::GetCurrentIndexBuffer(BindingIndexBuffer& buffer, bool& isDirtied)
{
	buffer = bindingIndexBuffer;
	isDirtied = isCurrentIndexBufferDirtied;
}

void CommandList::GetCurrentPipelineState(PipelineState*& pipelineState, bool& isDirtied)
{
	pipelineState = currentPipelineState;
	isDirtied = isPipelineDirtied;
}

void CommandList::GetCurrentComputeBuffer(int32_t unit, BindingComputeBuffer& buffer) { buffer = computeBuffers_[unit]; }

void CommandList::RegisterReferencedObject(ReferenceObject* referencedObject)
{
	if (referencedObject == nullptr)
		return;

	assert(swapIndex_ >= 0);
	SafeAddRef(referencedObject);
	swapObjects[swapIndex_].referencedObjects.push_back(referencedObject);
}

CommandList::CommandList(int32_t swapCount) : swapCount_(swapCount)
{
	constantBuffers_.fill(nullptr);

	for (auto& cbs : computeBuffers_)
	{
		cbs.computeBuffer = nullptr;
	}

	for (auto& t : currentTextures_)
	{
		t.texture = nullptr;
	}

	swapObjects.resize(swapCount_);
}

CommandList::~CommandList()
{
	for (auto& c : constantBuffers_)
	{
		SafeRelease(c);
	}

	for (auto& t : currentTextures_)
	{
		SafeRelease(t.texture);
	}

	for (auto& so : swapObjects)
	{
		for (auto& o : so.referencedObjects)
		{
			o->Release();
		}
		so.referencedObjects.clear();
	}

	for (auto& cb : computeBuffers_)
	{
		SafeRelease(cb.computeBuffer);
	}
}

void CommandList::Begin()
{
	bindingVertexBuffer.vertexBuffer = nullptr;
	bindingIndexBuffer.indexBuffer = nullptr;
	currentPipelineState = nullptr;
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
	ResetTextures();
	ResetComputeBuffer();

	swapIndex_ = (swapIndex_ + 1) % swapCount_;

	for (auto& o : swapObjects[swapIndex_].referencedObjects)
	{
		o->Release();
	}
	swapObjects[swapIndex_].referencedObjects.clear();

	isInBegin_ = true;
}

bool CommandList::BeginWithPlatform(void* platformContextPtr)
{
	bindingVertexBuffer.vertexBuffer = nullptr;
	bindingIndexBuffer.indexBuffer = nullptr;
	currentPipelineState = nullptr;
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
	ResetTextures();
	ResetComputeBuffer();

	swapIndex_ = (swapIndex_ + 1) % swapCount_;

	for (auto& o : swapObjects[swapIndex_].referencedObjects)
	{
		o->Release();
	}
	swapObjects[swapIndex_].referencedObjects.clear();
	doesBeginWithPlatform_ = true;

	isInBegin_ = true;
	return true;
}

void CommandList::End()
{
	isInBegin_ = false;

	if (GetIsInRenderPass())
	{
		Log(LogType::Error, "Please call End outside of RenderPass");
	}

	if (doesBeginWithPlatform_)
	{
		Log(LogType::Error, "CommandList begins with platform.");
	}
}

void CommandList::EndWithPlatform()
{
	isInBegin_ = false;

	if (!doesBeginWithPlatform_)
	{
		Log(LogType::Error, "CommandList doesn't begin with platform.");
	}
	doesBeginWithPlatform_ = false;
}

void CommandList::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) {}

void CommandList::Draw(int32_t primitiveCount, int32_t instanceCount)
{
	isVertexBufferDirtied = false;
	isCurrentIndexBufferDirtied = false;
	isPipelineDirtied = false;
}

void CommandList::SetVertexBuffer(Buffer* vertexBuffer, int32_t stride, int32_t offset)
{
	isVertexBufferDirtied |=
		bindingVertexBuffer.vertexBuffer != vertexBuffer || bindingVertexBuffer.stride != stride || bindingVertexBuffer.offset != offset;
	bindingVertexBuffer.vertexBuffer = vertexBuffer;
	bindingVertexBuffer.stride = stride;
	bindingVertexBuffer.offset = offset;

	RegisterReferencedObject(vertexBuffer);
}

void CommandList::SetIndexBuffer(Buffer* indexBuffer, int32_t stride, int32_t offset)
{
	isCurrentIndexBufferDirtied |= bindingIndexBuffer.indexBuffer != indexBuffer || bindingIndexBuffer.offset != offset;
	bindingIndexBuffer.indexBuffer = indexBuffer;
	bindingIndexBuffer.stride = stride;
	bindingIndexBuffer.offset = offset;

	RegisterReferencedObject(indexBuffer);
}

void CommandList::SetPipelineState(PipelineState* pipelineState)
{
	currentPipelineState = pipelineState;
	isPipelineDirtied = true;

	RegisterReferencedObject(pipelineState);
}

void CommandList::SetConstantBuffer(Buffer* constantBuffer, int32_t unit)
{
	SafeAssign(constantBuffers_[unit], constantBuffer);

	RegisterReferencedObject(constantBuffer);
}

void CommandList::SetComputeBuffer(Buffer* computeBuffer, int32_t stride, int32_t unit)
{
	SafeAssign(computeBuffers_[unit].computeBuffer, computeBuffer);
	computeBuffers_[unit].stride = stride;
	RegisterReferencedObject(computeBuffer);
}

void CommandList::SetTexture(Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit)
{
	SafeAssign(currentTextures_[unit].texture, texture);
	currentTextures_[unit].wrapMode = wrapMode;
	currentTextures_[unit].minMagFilter = minmagFilter;

	RegisterReferencedObject(texture);
}

void CommandList::ResetTextures()
{
	for (auto& texture : currentTextures_)
	{
		SafeRelease(texture.texture);
		texture.wrapMode = TextureWrapMode::Clamp;
		texture.minMagFilter = TextureMinMagFilter::Nearest;
	}
}

void CommandList::BeginRenderPass(RenderPass* renderPass)
{
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
	isInRenderPass_ = true;
}

bool CommandList::BeginRenderPassWithPlatformPtr(void* platformPtr)
{
	isVertexBufferDirtied = true;
	isCurrentIndexBufferDirtied = true;
	isPipelineDirtied = true;
	isInRenderPass_ = true;
	return true;
}

void CommandList::Dispatch(int32_t groupX, int32_t groupY, int32_t groupZ, int32_t threadX, int32_t threadY, int32_t threadZ)
{
	isPipelineDirtied = false;
}

void CommandList::ResetComputeBuffer()
{
	for (auto& cb : computeBuffers_)
	{
		SafeRelease(cb.computeBuffer);
		cb.stride = 0;
	}
}

void CommandList::SetImageData2D(Texture* texture, int32_t x, int32_t y, int32_t width, int32_t height, const void* data)
{
	assert(0); // TODO: Not implemented.
}

void CommandList::WaitUntilCompleted()
{
	assert(0); // TODO: Not implemented.
}

bool CommandList::GetIsInRenderPass() const { return isInRenderPass_; }

} // namespace LLGI
