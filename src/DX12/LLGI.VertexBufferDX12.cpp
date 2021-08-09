
#include "LLGI.VertexBufferDX12.h"
#include "../LLGI.VertexBuffer.h"

namespace LLGI
{

VertexBufferDX12::VertexBufferDX12() {}

VertexBufferDX12::~VertexBufferDX12() {
	SafeRelease(vertexBuffer_);
	SafeRelease(cpuVertexBuffer_);
}

bool VertexBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	vertexBuffer_ = graphics->CreateResource(
		D3D12_HEAP_TYPE_DEFAULT, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_STATE_GENERIC_READ, Vec2I(size, 1));

	if (vertexBuffer_ == nullptr)
		return false;

	cpuVertexBuffer_ = graphics->CreateResource(
		D3D12_HEAP_TYPE_UPLOAD, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_STATE_GENERIC_READ, Vec2I(size, 1));

	if (cpuVertexBuffer_ == nullptr)
	{
		SafeRelease(vertexBuffer_);
		return false;
	}

	size_ = size;
	return true;
}

void* VertexBufferDX12::Lock()
{
	auto hr = cpuVertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ : nullptr;
}

void* VertexBufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = cpuVertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset : nullptr;
}

void VertexBufferDX12::Unlock()
{
	cpuVertexBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

int32_t VertexBufferDX12::GetSize() { return size_; }

} // namespace LLGI
