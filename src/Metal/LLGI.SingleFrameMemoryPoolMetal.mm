#include "LLGI.GraphicsMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#include "LLGI.ConstantBufferMetal.h"
#import <MetalKit/MetalKit.h>

namespace LLGI
{

    InternalSingleFrameMemoryPoolMetal::InternalSingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount)
    {
		constantBufferSize_ = (constantBufferPoolSize + 255) & ~255; // buffer size should be multiple of 256

        buffer_ = new BufferMetal();
        buffer_->Initialize(graphics, constantBufferSize_);
    }
    InternalSingleFrameMemoryPoolMetal::~InternalSingleFrameMemoryPoolMetal()
    {
            SafeRelease(buffer_);
    }
    
    bool InternalSingleFrameMemoryPoolMetal::GetConstantBuffer(int32_t size, BufferMetal*& buffer, int32_t& offset)
    {
        if (constantBufferOffset_ + size > constantBufferSize_)
            return false;
        
        buffer = buffer_;
        offset = constantBufferOffset_;
        constantBufferOffset_ += size;
        return true;
    }
    
    void InternalSingleFrameMemoryPoolMetal::Reset()
    {
        constantBufferOffset_ = 0;
    }
    
SingleFrameMemoryPoolMetal::SingleFrameMemoryPoolMetal(GraphicsMetal* graphics, bool isStrongRef, int32_t constantBufferPoolSize, int32_t drawingCount)
    : graphics_(graphics), isStrongRef_(isStrongRef), drawingCount_(drawingCount)
{
    if (isStrongRef)
    {
        SafeAddRef(graphics_);
    }
    
    // TODO improve it
    int swapBufferCount = 3;
    
    for (size_t i = 0; i < swapBufferCount; i++)
    {
        auto memoryPool = std::make_shared<InternalSingleFrameMemoryPoolMetal>(graphics, constantBufferPoolSize, drawingCount);
        memoryPools.push_back(memoryPool);
    }
    
    currentSwap_ = -1;
}
    
SingleFrameMemoryPoolMetal::~SingleFrameMemoryPoolMetal()
{
    memoryPools.clear();
    
    if (isStrongRef_)
    {
        SafeRelease(graphics_);
    }
}
    
void SingleFrameMemoryPoolMetal::NewFrame()
{
    currentSwap_++;
    currentSwap_ %= memoryPools.size();
    memoryPools[currentSwap_]->Reset();
}
    
ConstantBuffer* SingleFrameMemoryPoolMetal::CreateConstantBuffer(int32_t size)
{
    int32_t offset = 0;
    BufferMetal* buffer = nullptr;
    if(memoryPools[currentSwap_]->GetConstantBuffer(size, buffer, offset))
    {
        auto obj = new ConstantBufferMetal();
        if (!obj->InitializeAsShortTime(buffer, size, offset))
        {
            SafeRelease(obj);
            return nullptr;
        }
        
        return obj;
    }
    
    return nullptr;
}
    
}
