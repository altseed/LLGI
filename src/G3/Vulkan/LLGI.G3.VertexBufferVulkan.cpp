#include "LLGI.G3.VertexBufferVulkan.h"

namespace LLGI
{
namespace G3
{

bool VertexBufferVulkan::Initialize(GraphicsVulkan* graphics, int32_t size)
{

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));

	// create a buffer on cpu
	{
		vk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.size = size;
		vertexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		cpuBuf->buffer = graphics_->GetDevice().createBuffer(vertexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(cpuBuf->buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		cpuBuf->devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(cpuBuf->buffer, cpuBuf->devMem, 0);
	}

	// create a buffer on gpu
	{
		vk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.size = size;
		vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		gpuBuf->buffer = graphics_->GetDevice().createBuffer(vertexBufferInfo);

		vk::MemoryRequirements memReqs = graphics_->GetDevice().getBufferMemoryRequirements(gpuBuf->buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics_->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		gpuBuf->devMem = graphics_->GetDevice().allocateMemory(memAlloc);
		graphics_->GetDevice().bindBufferMemory(gpuBuf->buffer, gpuBuf->devMem, 0);
	}

	memSize = size;

	return true;
}

VertexBufferVulkan::VertexBufferVulkan() {}

VertexBufferVulkan ::~VertexBufferVulkan() {}

void* VertexBufferVulkan::Lock()
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem, 0, memSize, vk::MemoryMapFlags());
	return data;
}

void* VertexBufferVulkan::Lock(int32_t offset, int32_t size)
{
	data = graphics_->GetDevice().mapMemory(cpuBuf->devMem, offset, size, vk::MemoryMapFlags());
	return data;
}

void VertexBufferVulkan::Unlock() { 
	
	graphics_->GetDevice().unmapMemory(cpuBuf->devMem); 

	// copy buffer
	vk::CommandBufferAllocateInfo cmdBufInfo;
	cmdBufInfo.commandPool = graphics_->GetCommandPool();
	cmdBufInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdBufInfo.commandBufferCount = 1;
	vk::CommandBuffer copyCommandBuffer = graphics_->GetDevice().allocateCommandBuffers(cmdBufInfo)[0];

	vk::CommandBufferBeginInfo cmdBufferBeginInfo;

	copyCommandBuffer.begin(cmdBufferBeginInfo);

	vk::BufferCopy copyRegion;
	copyRegion.size = memSize;
	copyCommandBuffer.copyBuffer(cpuBuf->buffer, gpuBuf->buffer, copyRegion);
	
	copyCommandBuffer.end();

	// submit and wait to execute command
	vk::SubmitInfo copySubmitInfo;
	copySubmitInfo.commandBufferCount = 1;
	copySubmitInfo.pCommandBuffers = &copyCommandBuffer;

	graphics_->GetQueue().submit(copySubmitInfo, VK_NULL_HANDLE);
	graphics_->GetQueue().waitIdle();
	
	graphics_->GetDevice().freeCommandBuffers(graphics_->GetCommandPool(), copyCommandBuffer);
}

int32_t VertexBufferVulkan::GetSize() { return memSize; }

} // namespace G3
} // namespace LLGI