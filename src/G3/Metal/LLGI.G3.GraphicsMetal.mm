#include "LLGI.G3.GraphicsMetal.h"
#include "LLGI.G3.Metal_Impl.h"
#include "LLGI.G3.CommandListMetal.h"
#import <MetalKit/MetalKit.h>

namespace LLGI {
namespace G3 {
    
Graphics_Impl::Graphics_Impl()
{
        
}
    
Graphics_Impl::~Graphics_Impl()
{
    if(device != nullptr)
    {
        [device release];
    }
    
    if(commandQueue != nullptr)
    {
        [commandQueue release];
    }
}

bool Graphics_Impl::Initialize()
{
    device =  MTLCreateSystemDefaultDevice();
    commandQueue = [device newCommandQueue];
    return true;
}
    
void Graphics_Impl::Execute(CommandList_Impl* commandBuffer)
{
    [commandBuffer->commandBuffer commit];
}
    
bool RenderPass_Impl::Initialize()
{
    renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
    return true;
}
    
GraphicsMetal::GraphicsMetal()
{
    impl = new Graphics_Impl();
}
    
GraphicsMetal::~GraphicsMetal()
{
    SafeDelete(impl);
}

bool GraphicsMetal::Initialize()
{
    return impl->Initialize();
}
    
void GraphicsMetal::NewFrame()
{
        // None
    }
    
    void GraphicsMetal::SetWindowSize(const Vec2I& windowSize)
    {
        throw "Not inplemented";
    }
    
    void GraphicsMetal::Execute(CommandList* commandList)
    {
        auto commandList_ = (CommandListMetal*)commandList;
        impl->Execute(commandList_->GetImpl());
    }
    
    void GraphicsMetal::WaitFinish()
    {
        throw "Not inplemented";
    }
    
    RenderPass* GraphicsMetal::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
    {
        throw "Not inplemented";
    }
    
    VertexBuffer* GraphicsMetal::CreateVertexBuffer(int32_t size)
    {
        throw "Not inplemented";
    }
    
    IndexBuffer* GraphicsMetal::CreateIndexBuffer(int32_t stride, int32_t count)
    {
     throw "Not inplemented";
    }
    
    Shader* GraphicsMetal::CreateShader(DataStructure* data, int32_t count)
    {
        throw "Not inplemented";
    }
    
    PipelineState* GraphicsMetal::CreatePiplineState()
    {
        throw "Not inplemented";
    }
    
    CommandList* GraphicsMetal::CreateCommandList()
    {
        auto commandList = new CommandListMetal();
        if(commandList->Initialize(this))
        {
            return commandList;
        }
        
        SafeRelease(commandList);
        return nullptr;
    }
    
    ConstantBuffer* GraphicsMetal::CreateConstantBuffer(int32_t size, ConstantBufferType type)
    {
        throw "Not inplemented";
    }
    
    RenderPass* GraphicsMetal::CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture)
    {
        throw "Not inplemented";
    }
    
    Texture* GraphicsMetal::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
    {
        throw "Not inplemented";
    }
    
    Texture* GraphicsMetal::CreateTexture(uint64_t id)
    {
        throw "Not inplemented";
    }
    
    Graphics_Impl* GraphicsMetal::GetImpl() const
    {
        return impl;
    }
}
}
