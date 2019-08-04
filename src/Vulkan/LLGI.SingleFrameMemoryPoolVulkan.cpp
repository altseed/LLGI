#include "LLGI.SingleFrameMemoryPoolVulkan.h"
#include "LLGI.ConstantBufferVulkan.h"

namespace LLGI
{

InternalSingleFrameMemoryPoolVulkan::InternalSingleFrameMemoryPoolVulkan(GraphicsVulkan* graphics,
																	 int32_t constantBufferPoolSize,
																	 int32_t drawingCount)
{
	constantBufferSize_ = (constantBufferPoolSize + 255) & ~255; // buffer size should be multiple of 256
    throw "Not implemented.";
}

InternalSingleFrameMemoryPoolVulkan ::~InternalSingleFrameMemoryPoolVulkan()
{
}

bool InternalSingleFrameMemoryPoolVulkan::GetConstantBuffer(int32_t size, VkBuffer** outResource, int32_t* outOffset)
{
	if (constantBufferOffset_ + size > constantBufferSize_)
		return false;

    throw "Not implemented.";
	//*outResource = ;
	*outOffset = constantBufferOffset_;
	constantBufferOffset_ += size;
	return true;
}

void InternalSingleFrameMemoryPoolVulkan::Reset()
{
    constantBufferOffset_ = 0;
}

SingleFrameMemoryPoolVulkan::SingleFrameMemoryPoolVulkan(
	GraphicsVulkan* graphics, bool isStrongRef, int32_t swapBufferCount, int32_t constantBufferPoolSize, int32_t drawingCount)
	: graphics_(graphics), isStrongRef_(isStrongRef), drawingCount_(drawingCount)
{
	if (isStrongRef)
	{
		SafeAddRef(graphics_);
	}

	for (size_t i = 0; i < swapBufferCount; i++)
	{
		auto memoryPool = std::make_shared<InternalSingleFrameMemoryPoolVulkan>(graphics, constantBufferPoolSize, drawingCount);
		memoryPools.push_back(memoryPool);
	}

	currentSwap_ = -1;
}

SingleFrameMemoryPoolVulkan ::~SingleFrameMemoryPoolVulkan()
{
	memoryPools.clear();

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

bool SingleFrameMemoryPoolVulkan::GetConstantBuffer(int32_t size, VkBuffer** outResource, int32_t* outOffset)
{
	assert(currentSwap_ >= 0);
	return memoryPools[currentSwap_]->GetConstantBuffer(size, outResource, outOffset);
}

InternalSingleFrameMemoryPoolVulkan* SingleFrameMemoryPoolVulkan::GetInternal()
{
    return memoryPools[currentSwap_].get();
}

int32_t SingleFrameMemoryPoolVulkan::GetDrawingCount() const { return drawingCount_; }

void SingleFrameMemoryPoolVulkan::NewFrame()
{
	currentSwap_++;
	currentSwap_ %= memoryPools.size();
	memoryPools[currentSwap_]->Reset();
}

ConstantBuffer* SingleFrameMemoryPoolVulkan::CreateConstantBuffer(int32_t size)
{
	auto obj = new ConstantBufferVulkan();
	if (!obj->InitializeAsShortTime(this, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}
} // namespace LLGI