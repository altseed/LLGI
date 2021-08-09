#include "LLGI.ConstantBufferVulkan.h"
#include "LLGI.SingleFrameMemoryPoolVulkan.h"

namespace LLGI
{

ConstantBufferVulkan::ConstantBufferVulkan() {}

ConstantBufferVulkan::~ConstantBufferVulkan() {}

bool ConstantBufferVulkan::Initialize(GraphicsVulkan* graphics, int32_t size)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	auto allocatedSize = GetAlignedSize(size, 256);

	memSize_ = size;

	// create a buffer on cpu
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = allocatedSize;
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
		IndexBufferInfo.size = allocatedSize;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		gpuBuf->Attach(buffer, devMem);
	}

	return true;
}

bool ConstantBufferVulkan::InitializeAsShortTime(GraphicsVulkan* graphics, SingleFrameMemoryPoolVulkan* memoryPool, int32_t size)
{
	if (cpuBuf == nullptr || gpuBuf == nullptr)
	{
		SafeAddRef(graphics);
		graphics_ = CreateSharedPtr(graphics);

		if (cpuBuf == nullptr)
			cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics_.get()));

		if (gpuBuf == nullptr)
			gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics_.get()));
	}

	auto alignedSize = static_cast<int32_t>(GetAlignedSize(size, 256));
	vk::Buffer buffer;
	vk::DeviceMemory deviceMemory;
	vk::Buffer cpuBuffer;
	vk::DeviceMemory cpuDeviceMemory;
	if (memoryPool->GetConstantBuffer(alignedSize, &buffer, &deviceMemory, &cpuBuffer, &cpuDeviceMemory, &offset_))
	{
		gpuBuf->Attach(buffer, deviceMemory, true);
		cpuBuf->Attach(cpuBuffer, cpuDeviceMemory, true);
		memSize_ = size;

		return true;
	}
	else
	{
		return false;
	}
}

void* ConstantBufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem(), offset_, memSize_, vk::MemoryMapFlags());
	return data;
}

void* ConstantBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem(), offset_ + offset, size, vk::MemoryMapFlags());
	return data;
}

void ConstantBufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(cpuBuf->devMem()); }

int32_t ConstantBufferVulkan::GetSize() { return memSize_; }

} // namespace LLGI
