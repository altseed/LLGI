#include "LLGI.BufferWebGPU.h"

namespace LLGI
{

bool BufferWebGPU::Initialize(wgpu::Device& device, const BufferUsageType usage, const int32_t size)
{
	wgpu::BufferDescriptor desc{};
	desc.size = size;

	if ((usage & BufferUsageType::Vertex) == BufferUsageType::Vertex)
	{
		desc.usage |= wgpu::BufferUsage ::Vertex;
	}

	if ((usage & BufferUsageType::Index) == BufferUsageType::Index)
	{
		desc.usage |= wgpu::BufferUsage::Index;
	}

	if ((usage & BufferUsageType::Index) == BufferUsageType::Constant)
	{
		desc.usage |= wgpu::BufferUsage::Uniform;
	}

	if ((usage & BufferUsageType::Index) == BufferUsageType::Compute)
	{
		desc.usage |= wgpu::BufferUsage::Storage;
	}

	buffer_ = device.CreateBuffer(&desc);
	size_ = size;
	usage_ = usage;
	return buffer_ != nullptr;
}

void* BufferWebGPU::Lock() { return buffer_.GetMappedRange(0, GetSize()); }

void* BufferWebGPU::Lock(int32_t offset, int32_t size) { return buffer_.GetMappedRange(offset, size); }

void BufferWebGPU::Unlock() { buffer_.Unmap(); }

int32_t BufferWebGPU::GetSize() { return size_; }

wgpu::Buffer& BufferWebGPU::GetBuffer() { return buffer_; }

} // namespace LLGI