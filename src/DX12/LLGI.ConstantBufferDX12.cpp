
#include "LLGI.ConstantBufferDX12.h"
#include "../LLGI.ConstantBuffer.h"
#include "LLGI.SingleFrameMemoryPoolDX12.h"

namespace LLGI
{

ConstantBufferDX12::ConstantBufferDX12() {}

ConstantBufferDX12::~ConstantBufferDX12() {
	SafeRelease(constantBuffer_);
	SafeRelease(cpuConstantBuffer_);
}

bool ConstantBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	memSize_ = size;
	actualSize_ = (size + 255) & ~255; // buffer size should be multiple of 256
	constantBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											   DXGI_FORMAT_UNKNOWN,
											   D3D12_RESOURCE_DIMENSION_BUFFER,
											   D3D12_RESOURCE_STATE_GENERIC_READ,
											   Vec2I(actualSize_, 1));
	if (constantBuffer_ == nullptr)
		return false;

	cpuConstantBuffer_ = graphics->CreateResource(D3D12_HEAP_TYPE_UPLOAD,
											   DXGI_FORMAT_UNKNOWN,
											   D3D12_RESOURCE_DIMENSION_BUFFER,
											   D3D12_RESOURCE_STATE_GENERIC_READ,
											   Vec2I(actualSize_, 1));

	if (cpuConstantBuffer_ == nullptr)
	{
		SafeRelease(constantBuffer_);
		return false;
	}

	return true;
}

bool ConstantBufferDX12::InitializeAsShortTime(SingleFrameMemoryPoolDX12* memoryPool, int32_t size)
{
	auto old = constantBuffer_;
	auto oldCpu = cpuConstantBuffer_;

	auto size_ = (size + 255) & ~255; // buffer size should be multiple of 256
	if (memoryPool->GetConstantBuffer(size_, constantBuffer_, offset_))
	{
		SafeAddRef(constantBuffer_);
		SafeRelease(old);
		memSize_ = size;
		actualSize_ = size_;

		if (memoryPool->GetCpuConstantBuffer(size_, cpuConstantBuffer_, offset_))
		{
			SafeAddRef(cpuConstantBuffer_);
			SafeRelease(oldCpu);
			memSize_ = size;
			actualSize_ = size_;
			return true;
		}
		else
		{
			SafeRelease(constantBuffer_);
			return false;
		}
	}
	else
	{
		return false;
	}
}

void* ConstantBufferDX12::Lock()
{
	auto hr = cpuConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? (mapped_ + offset_) : nullptr;
}

void* ConstantBufferDX12::Lock(int32_t offset, int32_t size)
{
	// TODO optimize it
	auto hr = cpuConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset_ + offset : nullptr;
}

void ConstantBufferDX12::Unlock()
{
	cpuConstantBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

int32_t ConstantBufferDX12::GetSize() { return memSize_; }

int32_t ConstantBufferDX12::GetActualSize() const { return actualSize_; }

int32_t ConstantBufferDX12::GetOffset() const { return offset_; }

} // namespace LLGI
