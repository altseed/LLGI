#include "LLGI.SingleFrameMemoryPoolVulkan.h"
#include "LLGI.ConstantBufferVulkan.h"

namespace LLGI
{

InternalSingleFrameMemoryPoolVulkan::InternalSingleFrameMemoryPoolVulkan() {}

InternalSingleFrameMemoryPoolVulkan ::~InternalSingleFrameMemoryPoolVulkan() {}

bool InternalSingleFrameMemoryPoolVulkan::Initialize(GraphicsVulkan* graphics, int32_t constantBufferPoolSize, int32_t drawingCount)
{
	constantBufferSize_ = (constantBufferPoolSize + 255) & ~255; // buffer size should be multiple of 256
	
	cpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));
	gpuBuf = std::unique_ptr<Buffer>(new Buffer(graphics));

	// create a buffer on gpu
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = constantBufferSize_;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
		vk::Buffer buffer = graphics->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::DeviceMemory devMem = graphics->GetDevice().allocateMemory(memAlloc);
		graphics->GetDevice().bindBufferMemory(buffer, devMem, 0);

		gpuBuf->Attach(buffer, devMem);
	}

	// create a buffer on cpu
	{
		vk::BufferCreateInfo IndexBufferInfo;
		IndexBufferInfo.size = constantBufferSize_;
		IndexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		vk::Buffer buffer = graphics->GetDevice().createBuffer(IndexBufferInfo);

		vk::MemoryRequirements memReqs = graphics->GetDevice().getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo memAlloc;
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = graphics->GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		vk::DeviceMemory devMem = graphics->GetDevice().allocateMemory(memAlloc);
		graphics->GetDevice().bindBufferMemory(buffer, devMem, 0);

		cpuBuf->Attach(buffer, devMem);
	}

	return true;
}

void InternalSingleFrameMemoryPoolVulkan::Dispose()
{
	gpuBuf = nullptr;
	cpuBuf = nullptr;
}

bool InternalSingleFrameMemoryPoolVulkan::GetConstantBuffer(int32_t size,
															vk::Buffer* outResource,
															vk::DeviceMemory* deviceMemory,
															vk::Buffer* outCpuResource,
															vk::DeviceMemory* cpuDeviceMemory,
															int32_t* outOffset)
{
	if (constantBufferOffset_ + size > constantBufferSize_)
		return false;

	*outResource = gpuBuf->buffer();
	*deviceMemory = gpuBuf->devMem();
	*outCpuResource = cpuBuf->buffer();
	*cpuDeviceMemory = cpuBuf->devMem();
	*outOffset = constantBufferOffset_;
	constantBufferOffset_ += size;
	return true;
}

void InternalSingleFrameMemoryPoolVulkan::Reset() { constantBufferOffset_ = 0; }

ConstantBuffer* SingleFrameMemoryPoolVulkan::CreateConstantBufferInternal(int32_t size)
{
	auto obj = new ConstantBufferVulkan();
	if (!obj->InitializeAsShortTime(graphics_, this, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

ConstantBuffer* SingleFrameMemoryPoolVulkan::ReinitializeConstantBuffer(ConstantBuffer* cb, int32_t size)
{
	auto obj = static_cast<ConstantBufferVulkan*>(cb);
	if (!obj->InitializeAsShortTime(graphics_, this, size))
	{
		return nullptr;
	}

	return obj;
}

SingleFrameMemoryPoolVulkan::SingleFrameMemoryPoolVulkan(
	GraphicsVulkan* graphics, bool isStrongRef, int32_t swapBufferCount, int32_t constantBufferPoolSize, int32_t drawingCount)
	: SingleFrameMemoryPool(swapBufferCount), graphics_(graphics), isStrongRef_(isStrongRef), currentSwap_(-1), drawingCount_(drawingCount)
{
	if (isStrongRef)
	{
		SafeAddRef(graphics_);
	}

	for (int32_t i = 0; i < swapBufferCount; i++)
	{
		auto memoryPool = std::make_shared<InternalSingleFrameMemoryPoolVulkan>();
		if (!memoryPool->Initialize(graphics, constantBufferPoolSize, drawingCount))
		{
			return;
		}
		memoryPools.push_back(memoryPool);
	}
}

SingleFrameMemoryPoolVulkan ::~SingleFrameMemoryPoolVulkan()
{
	for (auto& pool : memoryPools)
	{
		pool->Dispose();
	}
	memoryPools.clear();

	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

bool SingleFrameMemoryPoolVulkan::GetConstantBuffer(int32_t size,
													vk::Buffer* outResource,
													vk::DeviceMemory* deviceMemory,
													vk::Buffer* outCpuResource,
													vk::DeviceMemory* cpuDeviceMemory,
													int32_t* outOffset)
{
	assert(currentSwap_ >= 0);
	return memoryPools[currentSwap_]->GetConstantBuffer(size, outResource, deviceMemory, outCpuResource, cpuDeviceMemory, outOffset);
}

InternalSingleFrameMemoryPoolVulkan* SingleFrameMemoryPoolVulkan::GetInternal() { return memoryPools[currentSwap_].get(); }

int32_t SingleFrameMemoryPoolVulkan::GetDrawingCount() const { return drawingCount_; }

void SingleFrameMemoryPoolVulkan::NewFrame()
{
	currentSwap_++;
	currentSwap_ %= memoryPools.size();
	memoryPools[currentSwap_]->Reset();
	SingleFrameMemoryPool::NewFrame();
}

} // namespace LLGI
