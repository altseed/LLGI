#include "LLGI.GraphicsMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#import <MetalKit/MetalKit.h>

namespace LLGI
{

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
