#include "LLGI.ConstantBufferVulkan.h"

namespace LLGI
{

ConstantBufferVulkan::ConstantBufferVulkan() {}

ConstantBufferVulkan::~ConstantBufferVulkan() {}

bool ConstantBufferVulkan::Initialize(GraphicsVulkan* graphics, int32_t size)
{
	// TODO : shortTime
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	buffer = std::unique_ptr<Buffer>(new Buffer(graphics));
	memSize = size;
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = size;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
		buffer->buffer = graphics_->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(buffer->buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex =
			graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		buffer->devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(buffer->buffer, buffer->devMem, 0);
	}

	return true;
}

void* ConstantBufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(buffer->devMem, 0, memSize, vk::MemoryMapFlags());
	return data;
}

void* ConstantBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(buffer->devMem, offset, size, vk::MemoryMapFlags());
	return data;
}

void ConstantBufferVulkan::Unlock() { graphics_->GetDevice().unmapMemory(buffer->devMem); }

int32_t ConstantBufferVulkan::GetSize() { return memSize; }

} // namespace LLGI