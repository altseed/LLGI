
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "LLGI.G3.Metal_Impl.h"

namespace LLGI {
namespace G3{
    
    void Graphics_Impl::Execute(CommandBuffer_Impl* commandBuffer)
    {
        [commandBuffer->commandBuffer commit];
    }
    
    bool RenderPass_Impl::Initialize()
    {
        renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
        return true;
    }
    
    bool CommandBuffer_Impl::Initialize(Graphics_Impl* graphics)
    {
        commandBuffer = [graphics->commandQueue commandBuffer];
        return true;
    }
    
    void CommandBuffer_Impl::BeginRenderPass(RenderPass_Impl* renderPass)
    {
        renderEncoder = [commandBuffer
                         renderCommandEncoderWithDescriptor:renderPass->renderPassDescriptor];
    }
    
    void CommandBuffer_Impl::EndRenderPass()
    {
        if(renderEncoder)
        {
            [renderEncoder endEncoding];
            renderEncoder = nullptr;
        }
    }

    void PipelineState_Impl::Compile(Graphics_Impl* graphics)
    {
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        
        NSError *pipelineError = nil;
        pipelineState = [graphics->device
                         newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                         error:&pipelineError];
    }
    
    bool Buffer_Impl::Initialize(Graphics_Impl* graphics, int32_t size)
    {
        buffer = [graphics->device
                  newBufferWithBytes:NULL
                  length:size
                  options:MTLResourceOptionCPUCacheModeDefault];
        
        return true;
    }
    
    
    void* Buffer_Impl::GetBuffer()
    {
        return buffer.contents;
    }
}
}
