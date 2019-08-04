
#pragma once

#include "LLGI.BaseVulkan.h"

namespace LLGI
{
class GraphicsVulkan;

class InternalSingleFrameMemoryPoolVulkan
{
private:
	int32_t constantBufferSize_ = 0;
	int32_t constantBufferOffset_ = 0;

public:
    InternalSingleFrameMemoryPoolVulkan(GraphicsVulkan* graphics, int32_t constantBufferPoolSize, int32_t drawingCount);
	virtual ~InternalSingleFrameMemoryPoolVulkan();
	bool GetConstantBuffer(int32_t size, VkBuffer** outResource, int32_t* outOffset);
	void Reset();
};

class SingleFrameMemoryPoolVulkan : public SingleFrameMemoryPool
{
private:
    GraphicsVulkan* graphics_ = nullptr;
	bool isStrongRef_ = false;
	std::vector<std::shared_ptr<InternalSingleFrameMemoryPoolVulkan>> memoryPools;
	int32_t currentSwap_ = 0;
	int32_t drawingCount_ = 0;

public:
    SingleFrameMemoryPoolVulkan(
        GraphicsVulkan* graphics, bool isStrongRef, int32_t swapBufferCount, int32_t constantBufferPoolSize, int32_t drawingCount);
	virtual ~SingleFrameMemoryPoolVulkan();

	bool GetConstantBuffer(int32_t size, VkBuffer** outResource, int32_t* outOffset);

    InternalSingleFrameMemoryPoolVulkan* GetInternal();

	int32_t GetDrawingCount() const;

	void NewFrame() override;

	ConstantBuffer* CreateConstantBuffer(int32_t size) override;
};

} // namespace LLGI
