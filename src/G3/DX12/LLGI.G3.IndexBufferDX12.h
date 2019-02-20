
#pragma once

#include "../LLGI.G3.IndexBuffer.h"
#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{
namespace G3
{

class IndexBufferDX12 : public IndexBuffer
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;

	ID3D12Resource* indexBuffer = nullptr;

public:
	bool Initialize(GraphicsDX12* graphics, int32_t size);

	IndexBufferDX12();
	virtual ~IndexBufferDX12() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetStride();
	virtual int32_t GetCount();
};

} // namespace G3
} // namespace LLGI
