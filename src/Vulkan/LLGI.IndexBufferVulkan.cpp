#include "LLGI.IndexBufferVulkan.h"

namespace LLGI
{

bool IndexBufferVulkan::Initialize(GraphicsVulkan* graphics, int32_t stride, int32_t count)
{
	stride_ = stride;
	count_ = count;
	memSize = count_ * stride_;

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));

	// create a buffer on cpu
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = memSize;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		cpuBuf->Attach(buffer, devMem);
	}

	// create a buffer on gpu
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = memSize;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		gpuBuf->Attach(buffer, devMem);
	}

	return true;
}

IndexBufferVulkan::IndexBufferVulkan() {}

IndexBufferVulkan ::~IndexBufferVulkan() {}

void* IndexBufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem(), 0, memSize, vk::MemoryMapFlags());
	return data;
}

void* IndexBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem(), offset, size, vk::MemoryMapFlags());
	return data;
}

void IndexBufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(cpuBuf->devMem()); }

int32_t IndexBufferVulkan::GetStride() { return stride_; }

int32_t IndexBufferVulkan::GetCount() { return count_; }

} // namespace LLGI
