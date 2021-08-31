#include "LLGI.ComputeBufferDX12.h"

namespace LLGI
{
bool ComputeBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	memSize_ = size;
	actualSize_ = (size + 255) & ~255; // buffer size should be multiple of 256
	computeBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											  DXGI_FORMAT_UNKNOWN,
											  D3D12_RESOURCE_DIMENSION_BUFFER,
											  D3D12_RESOURCE_STATE_GENERIC_READ,
											  Vec2I(actualSize_, 1));
	if (computeBuffer_ == nullptr)
		return false;

	cpuComputeBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_UPLOAD,
												 DXGI_FORMAT_UNKNOWN,
												 D3D12_RESOURCE_DIMENSION_BUFFER,
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 Vec2I(actualSize_, 1));

	if (cpuComputeBuffer_ == nullptr)
	{
		SafeRelease(computeBuffer_);
		return false;
	}

	return true;
}

void* ComputeBufferDX12::Lock()
{
	auto hr = cpuComputeBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? (mapped_ + offset_) : nullptr;
}

void* ComputeBufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = cpuComputeBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset_ + offset : nullptr;
}

void ComputeBufferDX12::Unlock()
{
	cpuComputeBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

int32_t ComputeBufferDX12::GetSize() { return memSize_; }

int32_t ComputeBufferDX12::GetActualSize() const { return actualSize_; }

int32_t ComputeBufferDX12::GetOffset() const { return offset_; }

} // namespace LLGI
