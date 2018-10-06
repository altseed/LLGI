
#pragma once

#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{
namespace G3
{

	struct Vertex3D {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT4 Color;
	};

class VertexBufferDX12
	: public ReferenceObject
{
	private:
		std::shared_ptr<GraphicsDX12> graphics_;

		ID3D12Resource* vertexBuffer = nullptr;
		D3D12_HEAP_PROPERTIES heapProperties;
		D3D12_RESOURCE_DESC   resourceDesc;
		Vertex3D* buffer;

		bool Initialize(GraphicsDX12* graphics);

	public:

	VertexBufferDX12(GraphicsDX12* graphics);
	virtual ~VertexBufferDX12() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetSize();
};

}
}