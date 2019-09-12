#include "LLGI.GraphicsMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#import <MetalKit/MetalKit.h>

namespace LLGI
{

    InternalSingleFrameMemoryPoolMetal::InternalSingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount)
    {
        buffer_ = new BufferMetal();
        buffer_->Initialize(graphics, constantBufferSize_);
    }
    InternalSingleFrameMemoryPoolMetal::~InternalSingleFrameMemoryPoolMetal()
    {
            SafeRelease(buffer_);
    }
    
    bool InternalSingleFrameMemoryPoolMetal::GetConstantBuffer(int32_t size, Buffer_Impl*& buffer, int32_t& offset)
    {
        return false;
    }
    
    void InternalSingleFrameMemoryPoolMetal::Reset()
    {
        constantBufferOffset_ = 0;
    }
    
SingleFrameMemoryPoolMetal::SingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount)
{

}
    
SingleFrameMemoryPoolMetal::~SingleFrameMemoryPoolMetal()
{

}
    
void SingleFrameMemoryPoolMetal::NewFrame()
{
    printf("Warning: Not implemented.¥n");
}
    
ConstantBuffer* SingleFrameMemoryPoolMetal::CreateConstantBuffer(int32_t size)
{
    printf("Warning: Not implemented.¥n");
}
    
}
