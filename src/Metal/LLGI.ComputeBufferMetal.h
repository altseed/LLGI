#pragma once

#include "../LLGI.ComputeBuffer.h"
#include "LLGI.BufferMetal.h"

namespace LLGI
{

class ComputeBufferMetal : public ComputeBuffer
{
private:
	BufferMetal* buffer_ = nullptr;
	int32_t size_ = 0;
	int32_t offset_ = 0;

public:
	ComputeBufferMetal();
	~ComputeBufferMetal() override;

	bool Initialize(Graphics* graphics, int32_t size);

	void* Lock() override;

	void* Lock(int32_t offset, int32_t size) override;

	void Unlock() override;

	void* const Read() override;

	int32_t GetSize() override;

	int32_t GetOffset() const { return offset_; }

	BufferMetal& GetBuffer() { return *buffer_; }
};

} // namespace LLGI
