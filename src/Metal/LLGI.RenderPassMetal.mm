#include "LLGI.GraphicsMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.ConstantBufferMetal.h"
#include "LLGI.IndexBufferMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.PipelineStateMetal.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.VertexBufferMetal.h"
#include "LLGI.TextureMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#include "LLGI.RenderPassMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
    
RenderPass_Impl::RenderPass_Impl() {}
    
RenderPass_Impl::~RenderPass_Impl()
{
    if (renderPassDescriptor != nullptr)
    {
        [renderPassDescriptor release];
        renderPassDescriptor = nullptr;
    }
}
    
bool RenderPass_Impl::Initialize()
{
	renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
	return true;
}

/*
void RenderPass_Impl::UpdateTarget(Graphics_Impl* graphics)
{
    renderPassDescriptor.colorAttachments[0].texture = graphics->drawable.texture;
    pixelFormat = graphics->drawable.texture.pixelFormat;
}
*/
    
void RenderPass_Impl::UpdateTarget(Texture_Impl** textures, int32_t textureCount, Texture_Impl* depthTexture)
{
    for(int i = 0; i < textureCount; i++)
    {
        renderPassDescriptor.colorAttachments[i].texture = textures[i]->texture;
    }
    
    if(depthTexture != nullptr)
    {
        renderPassDescriptor.depthAttachment.texture = depthTexture->texture;
		renderPassDescriptor.stencilAttachment.texture = depthTexture->texture;
		depthStencilFormat = depthTexture->texture.pixelFormat;
    }
    
    pixelFormat = textures[0]->texture.pixelFormat;
}

    
RenderPassMetal::RenderPassMetal()
{
    impl = new RenderPass_Impl();
	impl->Initialize();
}

RenderPassMetal::~RenderPassMetal()
{
    SafeDelete(impl);
}

void RenderPassMetal::UpdateRenderTarget(Texture** textures, int32_t textureCount, Texture* depthTexture)
{
    std::array<Texture_Impl*, 8> texturesImpl;
    texturesImpl.fill(nullptr);
    Texture_Impl* depthTextureImpl = nullptr;
    
    for(int32_t i = 0; i < textureCount; i++)
    {
        if(textures[i] == nullptr)
            continue;
        
        texturesImpl.at(i) = reinterpret_cast<TextureMetal*>(textures[i])->GetImpl();
        SafeAddRef(textures[i]);
        colorBuffers_.at(i) = CreateSharedPtr(reinterpret_cast<TextureMetal*>(textures[i]));
    }
    
    if(depthTexture != nullptr)
    {
        depthTextureImpl = reinterpret_cast<const TextureMetal*>(depthTexture)->GetImpl();
    }
    
    impl->UpdateTarget(texturesImpl.data(), textureCount, depthTextureImpl);
}
    
void RenderPassMetal::SetIsColorCleared(bool isColorCleared)
{
	impl->isColorCleared = isColorCleared;
	RenderPass::SetIsColorCleared(isColorCleared);
}

void RenderPassMetal::SetIsDepthCleared(bool isDepthCleared)
{
	impl->isDepthCleared = isDepthCleared;
	RenderPass::SetIsDepthCleared(isDepthCleared);
}

void RenderPassMetal::SetClearColor(const Color8& color)
{
	impl->clearColor = color;
	RenderPass::SetClearColor(color);
}

Texture* RenderPassMetal::GetColorBuffer(int index) { return colorBuffers_[index].get(); }
	
RenderPass_Impl* RenderPassMetal::GetImpl() const { return impl; }

RenderPassPipelineStateMetal::RenderPassPipelineStateMetal() { impl = new RenderPassPipelineState_Impl(); }

RenderPassPipelineStateMetal::~RenderPassPipelineStateMetal() { SafeDelete(impl); }

RenderPassPipelineState_Impl* RenderPassPipelineStateMetal::GetImpl() const { return impl; }
    
}
