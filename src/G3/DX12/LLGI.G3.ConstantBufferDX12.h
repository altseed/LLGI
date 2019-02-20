
#pragma once

#include "../LLGI.G3.ConstantBuffer.h"
#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{
namespace G3
{

class ConstantBufferDX12 : public ConstantBuffer
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;

	ID3D12Resource* constantBuffer = nullptr;

public:
	bool Initialize(GraphicsDX12* graphics, int32_t size);

	ConstantBufferDX12();
	virtual ~ConstantBufferDX12() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetSize();
};

} // namespace G3
} // namespace LLGI
