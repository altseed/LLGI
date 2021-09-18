#pragma once

#include "../LLGI.ComputeBuffer.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

class ComputeBufferVulkan : public ComputeBuffer
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::unique_ptr<Buffer> uplaodBuf;
	std::unique_ptr<Buffer> readbackBuf;
	std::unique_ptr<Buffer> gpuBuf;
	int memSize_ = 0;
	void* data = nullptr;
	void* readData = nullptr;
	int32_t offset_ = 0;

public:
	ComputeBufferVulkan();
	~ComputeBufferVulkan() override;

	bool Initialize(GraphicsVulkan* graphics, int32_t size);

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;

	void* const Read() override;

	int32_t GetSize() override;
	int32_t GetOffset() const { return offset_; }

	vk::Buffer GetBuffer() { return gpuBuf->buffer(); }
	vk::Buffer GetUploadBuffer() { return uplaodBuf->buffer(); }
	vk::Buffer GetReadbackBuffer() { return readbackBuf->buffer(); }
};

} // namespace LLGI
