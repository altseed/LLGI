#include "LLGI.BufferVulkan.h"

namespace LLGI
{

BufferVulkan::BufferVulkan() {}

BufferVulkan::~BufferVulkan() {}

bool BufferVulkan::Initialize(GraphicsVulkan* graphics, BufferUsageType usage, int32_t size)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	buffer_ = std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics));
	stagingBuffer_ = std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics));
	readbackBuffer_ = std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics));

	usage_ = usage;
	size_ = size;
	actualSize_ = size;
	vk::BufferUsageFlags vkUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

	if ((usage & BufferUsageType::Index) == BufferUsageType::Index)
	{
		vkUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
	}

	if ((usage & BufferUsageType::Vertex) == BufferUsageType::Vertex)
	{
		vkUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
	}

	if ((usage & BufferUsageType::Compute) == BufferUsageType::Compute)
	{
		vkUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
	}

	if ((usage & BufferUsageType::Constant) == BufferUsageType::Constant)
	{
		vkUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
		actualSize_ = static_cast<int32_t>(GetAlignedSize(size, 256)); // buffer size should be multiple of 256
	}

	// create a buffer on cpu
	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = actualSize_;
		ComputeBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		stagingBuffer_->Attach(buffer, devMem);
	}

	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = actualSize_;
		ComputeBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		readbackBuffer_->Attach(buffer, devMem);
	}

	// create a buffer on gpu
	{
		vk::BufferCreateInfo ComputeBufferInfo;
		ComputeBufferInfo.size = actualSize_;
		ComputeBufferInfo.usage = vkUsage;
		vk::Buffer buffer = graphics_->GetDevice().createBuffer(ComputeBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::DeviceMemory devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer, devMem, 0);

		buffer_->Attach(buffer, devMem);
	}

	return true;
}

bool BufferVulkan::InitializeAsShortTime(GraphicsVulkan* graphics, SingleFrameMemoryPoolVulkan* memoryPool, int32_t size)
{
	if (buffer_ == nullptr || readbackBuffer_ == nullptr || stagingBuffer_ == nullptr)
	{
		SafeAddRef(graphics);
		graphics_ = CreateSharedPtr(graphics);

		if (buffer_ == nullptr)
			buffer_= std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics_.get()));

		if (readbackBuffer_ == nullptr)
			readbackBuffer_ = std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics_.get()));

		if (stagingBuffer_ == nullptr)
			stagingBuffer_ = std::unique_ptr<InternalBuffer>(new InternalBuffer(graphics_.get()));
	}

	auto alignedSize = static_cast<int32_t>(GetAlignedSize(size, 256));
	BufferVulkan* poolBuffer;
	if (memoryPool->GetConstantBuffer(alignedSize, poolBuffer, offset_))
	{
		buffer_->Attach(poolBuffer->buffer_->buffer(), poolBuffer->buffer_->devMem(), true);
		stagingBuffer_->Attach(poolBuffer->stagingBuffer_->buffer(), poolBuffer->stagingBuffer_->devMem(), true);
		readbackBuffer_->Attach(poolBuffer->readbackBuffer_->buffer(), poolBuffer->readbackBuffer_->devMem(), true);
		size_ = size;
		actualSize_ = alignedSize;

		return true;
	}
	else
	{
		return false;
	}
}

void* BufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(stagingBuffer_->devMem(), offset_, actualSize_, vk::MemoryMapFlags());
	return data;
}

void* BufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(stagingBuffer_->devMem(), offset_ + offset, size, vk::MemoryMapFlags());
	return data;
}

void BufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(stagingBuffer_->devMem()); }

void* const BufferVulkan::Read()
{
	readData = graphics_->GetDevice().mapMemory(readbackBuffer_->devMem(), offset_, actualSize_, vk::MemoryMapFlags());
	graphics_->GetDevice().unmapMemory(readbackBuffer_->devMem());
	return readData;
}

int32_t BufferVulkan::GetSize() { return size_; }

} // namespace LLGI
