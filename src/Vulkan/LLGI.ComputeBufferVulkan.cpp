#include "LLGI.ComputeBufferVulkan.h"

namespace LLGI
{

ComputeBufferVulkan::ComputeBufferVulkan() {}

ComputeBufferVulkan::~ComputeBufferVulkan() {}

bool ComputeBufferVulkan::Initialize(GraphicsVulkan* graphics, int32_t size)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	uplaodBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	readbackBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	auto allocatedSize = GetAlignedSize(size, 256);

	memSize_ = size;

	// create a buffer on cpu
	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = allocatedSize;
		ComputeBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		uplaodBuf->Attach(buffer, devMem);
	}

	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = allocatedSize;
		ComputeBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		readbackBuf->Attach(buffer, devMem);
	}

	// create a buffer on gpu
	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = allocatedSize;
		ComputeBufferInfo.usage =
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

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

void* ComputeBufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(uplaodBuf->devMem(), offset_, memSize_, vk::MemoryMapFlags());
	return data;
}

void* ComputeBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(uplaodBuf->devMem(), offset_ + offset, size, vk::MemoryMapFlags());
	return data;
}

void ComputeBufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(uplaodBuf->devMem()); }

void* const ComputeBufferVulkan::Read()
{
	readData = graphics_->GetDevice().mapMemory(readbackBuf->devMem(), offset_, memSize_, vk::MemoryMapFlags());
	graphics_->GetDevice().unmapMemory(readbackBuf->devMem());
	return readData;
}

int32_t ComputeBufferVulkan::GetSize() { return memSize_; }

} // namespace LLGI
