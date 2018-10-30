#include "LLGI.G3.CommandListMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI {
namespace G3 {
    
struct CommandListMetal_Impl
{
    id<MTLCommandBuffer> commandBuffer;
    id <MTLRenderCommandEncoder> renderEncoder;
    
    void BeginRenderPass()
    {
        
    }
    
    void EndRenderPass()
    {
        if(renderEncoder != nullptr)
        {
            [renderEncoder endEncoding];
            [renderEncoder release];
            renderEncoder = nullptr;
        }
    }
};
    
}
}
