#pragma once

#include "../LLGI.Buffer.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{
/**
 * TODO : Implement short time buffer
*/
class BufferWebGPU : public Buffer
{
	wgpu::Buffer buffer_ = nullptr;
	int32_t size_ = 0;
	int32_t offset_ = 0;

public:
	bool Initialize(wgpu::Device& device, const BufferUsageType usage, const int32_t size);
	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;

	int32_t GetSize() override;

	int32_t GetOffset() const { return offset_; }

	wgpu::Buffer& GetBuffer();
};

} // namespace LLGI
