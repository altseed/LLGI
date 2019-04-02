
#pragma once

#include "../LLGI.VertexBuffer.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{

struct Vertex3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Color;
};

class VertexBufferDX12 : public VertexBuffer
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;

	ID3D12Resource* vertexBuffer = nullptr;

	Vertex3D* mapped;

public:
	bool Initialize(GraphicsDX12* graphics, int32_t size);

	VertexBufferDX12();
	virtual ~VertexBufferDX12() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetSize();
	ID3D12Resource* Get() { return vertexBuffer; }
};

} // namespace LLGI
