
#include "LLGI.IndexBufferDX12.h"
#include "../LLGI.IndexBuffer.h"

namespace LLGI
{

IndexBufferDX12::IndexBufferDX12() {}

IndexBufferDX12::~IndexBufferDX12()
{
	SafeRelease(indexBuffer_);
	SafeRelease(cpuIndexBuffer_);
}

bool IndexBufferDX12::Initialize(GraphicsDX12* graphics, int32_t stride, int32_t count)
{
	indexBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											DXGI_FORMAT_UNKNOWN,
											D3D12_RESOURCE_DIMENSION_BUFFER,
											D3D12_RESOURCE_STATE_GENERIC_READ,
											Vec2I(stride * count, 1));

	if (indexBuffer_ == nullptr)
		return false;

	cpuIndexBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_UPLOAD,
											   DXGI_FORMAT_UNKNOWN,
											   D3D12_RESOURCE_DIMENSION_BUFFER,
											   D3D12_RESOURCE_STATE_GENERIC_READ,
											   Vec2I(stride * count, 1));

	if (cpuIndexBuffer_ == nullptr)
	{
		SafeRelease(indexBuffer_);
		return false;
	}

	stride_ = stride;
	count_ = count;
	return true;
}

void* IndexBufferDX12::Lock()
{
	auto hr = cpuIndexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ : nullptr;
}

void* IndexBufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = cpuIndexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset : nullptr;
}

void IndexBufferDX12::Unlock()
{
	cpuIndexBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

} // namespace LLGI
