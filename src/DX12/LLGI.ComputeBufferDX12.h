#pragma once

#include "../LLGI.ComputeBuffer.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

namespace LLGI
{

class ComputeBufferDX12 : public ComputeBuffer
{
private:
	ID3D12Resource* computeBuffer_ = nullptr;
	ID3D12Resource* readbackComputeBuffer_ = nullptr;
	ID3D12Resource* uploadComputeBuffer_ = nullptr;
	int memSize_ = 0;
	int actualSize_ = 0;
	int offset_ = 0;
	uint8_t* mapped_ = nullptr;
	uint8_t* readMapped_ = nullptr;

public:
	bool Initialize(GraphicsDX12* graphics, int32_t size);

	ComputeBufferDX12() = default;
	~ComputeBufferDX12() override;

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;

	void* const Read() override;

	int32_t GetSize() override;

	/**
		@brief	memory size must be multiple of 256 in DirectX12. So specified size and actual size are different.
	*/
	int32_t GetActualSize() const;
	int32_t GetOffset() const;
	
	ID3D12Resource* Get() { return computeBuffer_; }
	ID3D12Resource* GetReadback() { return readbackComputeBuffer_; }
	ID3D12Resource* GetUpload() { return uploadComputeBuffer_; }
};

} // namespace LLGI
