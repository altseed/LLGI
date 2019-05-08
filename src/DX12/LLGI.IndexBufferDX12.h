
#pragma once

#include "../LLGI.IndexBuffer.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{

class IndexBufferDX12 : public IndexBuffer
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;

	ID3D12Resource* indexBuffer = nullptr;

	uint8_t* mapped = nullptr;
	int32_t stride = 0;
	int32_t count = 0;

public:
	bool Initialize(GraphicsDX12* graphics, int32_t stride, int32_t count);

	IndexBufferDX12();
	virtual ~IndexBufferDX12() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetStride() { return stride; }
	virtual int32_t GetCount() { return count; }
	ID3D12Resource* Get() { return indexBuffer; }
};

} // namespace LLGI
