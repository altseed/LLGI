
#pragma once

#include "../LLGI.Buffer.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{
class SingleFrameMemoryPoolVulkan;

class BufferVulkan : public Buffer
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::unique_ptr<InternalBuffer> buffer_;
	std::unique_ptr<InternalBuffer> stagingBuffer_;
	std::unique_ptr<InternalBuffer> readbackBuffer_;
	void* data = nullptr;
	void* readData = nullptr;
	int32_t size_ = 0;
	int32_t actualSize_ = 0;
	int32_t offset_ = 0;

public:
	bool Initialize(GraphicsVulkan* graphics, BufferUsageType usage, int32_t size);
	bool InitializeAsShortTime(GraphicsVulkan* graphics, SingleFrameMemoryPoolVulkan* memoryPool, int32_t size);

	BufferVulkan();
	~BufferVulkan() override;

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;

	void* const Read() override;

	int32_t GetSize() override;
	int32_t GetActualSize() const { return actualSize_; }
	int32_t GetOffset() const { return offset_; }

	vk::Buffer GetBuffer() { return buffer_->buffer(); }
	vk::Buffer GetStagingBuffer() { return stagingBuffer_->buffer(); }
	vk::Buffer GetReadbackBuffer() { return readbackBuffer_->buffer(); }
};

} // namespace LLGI
