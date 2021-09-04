#include "LLGI.ComputeBufferDX12.h"

namespace LLGI
{
bool ComputeBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	memSize_ = size;
	actualSize_ = size; // buffer size should be multiple of 256
	computeBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											  DXGI_FORMAT_UNKNOWN,
											  D3D12_RESOURCE_DIMENSION_BUFFER,
											  D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
											  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
											  Vec2I(actualSize_, 1));
	if (computeBuffer_ == nullptr)
		return false;

	uploadComputeBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_UPLOAD,
													DXGI_FORMAT_UNKNOWN,
													D3D12_RESOURCE_DIMENSION_BUFFER,
													D3D12_RESOURCE_STATE_GENERIC_READ,
													Vec2I(actualSize_, 1));

	if (uploadComputeBuffer_ == nullptr)
	{
		SafeRelease(computeBuffer_);
		return false;
	}

	readbackComputeBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_READBACK,
													  DXGI_FORMAT_UNKNOWN,
													  D3D12_RESOURCE_DIMENSION_BUFFER,
													  D3D12_RESOURCE_STATE_COPY_DEST,
													  Vec2I(actualSize_, 1));

	if (readbackComputeBuffer_ == nullptr)
	{
		SafeRelease(computeBuffer_);
		SafeRelease(uploadComputeBuffer_);
		return false;
	}

	return true;
}

ComputeBufferDX12::~ComputeBufferDX12() {
	SafeRelease(computeBuffer_);
	SafeRelease(uploadComputeBuffer_);
	SafeRelease(readbackComputeBuffer_);
}

void* ComputeBufferDX12::Lock()
{
	auto hr = uploadComputeBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? (mapped_ + offset_) : nullptr;
}

void* ComputeBufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = uploadComputeBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset_ + offset : nullptr;
}

void ComputeBufferDX12::Unlock()
{
	uploadComputeBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

void* const ComputeBufferDX12::Read()
{
	auto hr = readbackComputeBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&readMapped_));
	readbackComputeBuffer_->Unmap(0, nullptr);
	return SUCCEEDED(hr) ? (readMapped_ + offset_) : nullptr;
}

int32_t ComputeBufferDX12::GetSize() { return memSize_; }

int32_t ComputeBufferDX12::GetActualSize() const { return actualSize_; }

int32_t ComputeBufferDX12::GetOffset() const { return offset_; }

} // namespace LLGI
