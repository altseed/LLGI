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

	buffer_ = std::unique_ptr<Buffer>(new Buffer(graphics));
	memSize_ = size;
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = size;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
		buffer_->buffer_ = graphics_->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer_->buffer_);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		buffer_->devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer_->buffer_, buffer_->devMem, 0);
	}

	return true;
}

bool ConstantBufferVulkan::InitializeAsShortTime(GraphicsVulkan* graphics, SingleFrameMemoryPoolVulkan* memoryPool, int32_t size)
{
	if(buffer_ == nullptr)
	{
		SafeAddRef(graphics);
		graphics_ = CreateSharedPtr(graphics);
		buffer_ = std::unique_ptr<Buffer>(new Buffer(graphics_.get()));
	}

	auto size_ = (size + 255) & ~255; // buffer size should be multiple of 256
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;
	if (memoryPool->GetConstantBuffer(size_, &buffer, &deviceMemory, &offset_))
	{
		buffer_->buffer_ = vk::Buffer(buffer);
		buffer_->devMem = deviceMemory;
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
	data = graphics_->GetDevice().mapMemory(buffer_->devMem, offset_, memSize_, vk::MemoryMapFlags());
	return data;
}

void* ConstantBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(buffer_->devMem, offset_ + offset, size, vk::MemoryMapFlags());
	return data;
}

void ConstantBufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(buffer_->devMem); }

int32_t ConstantBufferVulkan::GetSize() { return memSize_; }

} // namespace LLGI