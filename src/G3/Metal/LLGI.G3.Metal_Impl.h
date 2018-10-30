#pragma once

#include "../LLGI.G3.Graphics.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
namespace G3
{
    
struct CommandBuffer_Impl;
    
struct Graphics_Impl
{
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    void Execute(CommandBuffer_Impl* commandBuffer);
};

struct RenderPass_Impl
{
    MTLRenderPassDescriptor* renderPassDescriptor;
    
    bool Initialize();
};
    
struct CommandBuffer_Impl
{
    id<MTLCommandBuffer> commandBuffer;
    id<MTLRenderCommandEncoder> renderEncoder;
    
    bool Initialize(Graphics_Impl* graphics);
    
    void BeginRenderPass(RenderPass_Impl* renderPass);
    void EndRenderPass();
};
    
struct PipelineState_Impl
{
    id <MTLRenderPipelineState> pipelineState;
    
    void Compile(Graphics_Impl* graphics);
};
    
struct Buffer_Impl
{
    id <MTLBuffer> buffer;
    
    bool Initialize(Graphics_Impl* graphics, int32_t size);
    
    void* GetBuffer();
};
    
}
}
