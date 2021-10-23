#include "LLGI.BufferDX12.h"

namespace LLGI
{

BufferDX12::BufferDX12()
{
	size_ = 0;
	state_ = D3D12_RESOURCE_STATE_COMMON;
}

BufferDX12::~BufferDX12() { SafeRelease(buffer_); }

bool BufferDX12::Initialize(GraphicsDX12* graphics, const BufferUsageType usage, const int32_t size)
{
	usage_ = usage;

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	size_ = size;

	if ((usage & BufferUsageType::Index) == BufferUsageType::Index)
	{
		state_ |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
	}

	if ((usage & BufferUsageType::Vertex) == BufferUsageType::Vertex || (usage & BufferUsageType::Constant) == BufferUsageType::Constant)
	{
		state_ |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}

	if ((usage & BufferUsageType::Compute) == BufferUsageType::Compute)
	{
		state_ |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	buffer_ = graphics->CreateResource(
		D3D12_HEAP_TYPE_DEFAULT, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, state_, flags, Vec2I(size, 1));

	if (buffer_ == nullptr)
		return false;

	stagingBuffer_ = graphics->CreateResource(
		D3D12_HEAP_TYPE_UPLOAD, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_STATE_GENERIC_READ, Vec2I(size, 1));

	if (stagingBuffer_ == nullptr)
	{
		SafeRelease(buffer_);
		return false;
	}

	readbackBuffer_ = graphics->CreateResource(
		D3D12_HEAP_TYPE_READBACK, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST, Vec2I(size, 1));

	if (readbackBuffer_ == nullptr)
	{
		SafeRelease(buffer_);
		SafeRelease(stagingBuffer_);
		return false;
	}

	return true;
}

void* BufferDX12::Lock()
{
	auto hr = stagingBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ : nullptr;
}

void* BufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = stagingBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	return SUCCEEDED(hr) ? mapped_ + offset : nullptr;
}

void BufferDX12::Unlock()
{
	stagingBuffer_->Unmap(0, nullptr);
	mapped_ = nullptr;
}

void* const BufferDX12::Read()
{
	auto hr = readbackBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&readMapped_));
	readbackBuffer_->Unmap(0, nullptr);
	return SUCCEEDED(hr) ? readMapped_ : nullptr;
}

int32_t BufferDX12::GetSize() { return size_; }

} // namespace LLGI
