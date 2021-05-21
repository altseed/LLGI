#include "LLGI.RenderPassMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.ConstantBufferMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.IndexBufferMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.PipelineStateMetal.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#include "LLGI.TextureMetal.h"
#include "LLGI.VertexBufferMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

void RenderPassMetal::UpdateTarget(TextureMetal** textures,
								   int32_t textureCount,
								   TextureMetal* depthTexture,
								   TextureMetal* resolvedTexture,
								   TextureMetal* resolvedDepthTexture)
{
	pixelFormats.resize(textureCount);

	for (int i = 0; i < textureCount; i++)
	{
		renderPassDescriptor.colorAttachments[i].texture = textures[i]->GetTexture();
		pixelFormats.at(i) = textures[i]->GetTexture().pixelFormat;

		if (resolvedTexture != nullptr)
		{
			renderPassDescriptor.colorAttachments[i].resolveTexture = resolvedTexture->GetTexture();
			renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionMultisampleResolve;
		}
	}

	if (depthTexture != nullptr)
	{
		renderPassDescriptor.depthAttachment.texture = depthTexture->GetTexture();

		if (resolvedDepthTexture != nullptr)
		{
			renderPassDescriptor.depthAttachment.resolveTexture = resolvedDepthTexture->GetTexture();
			renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
		}

		if (HasStencil(ConvertFormat(depthTexture->GetTexture().pixelFormat)))
		{
			renderPassDescriptor.stencilAttachment.texture = depthTexture->GetTexture();

			if (resolvedDepthTexture != nullptr)
			{
				renderPassDescriptor.stencilAttachment.resolveTexture = resolvedDepthTexture->GetTexture();
				renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionMultisampleResolve;
			}
		}

		depthStencilFormat = depthTexture->GetTexture().pixelFormat;
	}
}

RenderPassMetal::RenderPassMetal()
{
    renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
}

RenderPassMetal::~RenderPassMetal()
{
    if (renderPassDescriptor != nullptr)
    {
        [renderPassDescriptor release];
        renderPassDescriptor = nullptr;
    }
}

bool RenderPassMetal::UpdateRenderTarget(
	Texture** textures, int32_t textureCount, Texture* depthTexture, Texture* resolvedTexture, Texture* resolvedDepthTexture)
{
	if (!assignRenderTextures(textures, textureCount))
	{
		return false;
	}

	if (!assignDepthTexture(depthTexture))
	{
		return false;
	}

	if (!assignResolvedRenderTexture(resolvedTexture))
	{
		return false;
	}

	if (!assignResolvedDepthTexture(resolvedDepthTexture))
	{
		return false;
	}

	if (!getSize(screenSize_, (const Texture**)textures, textureCount, depthTexture))
	{
		return false;
	}

	std::array<TextureMetal*, RenderTargetMax> texturesImpl;
	texturesImpl.fill(nullptr);
	TextureMetal* depthTextureImpl = nullptr;

	for (int32_t i = 0; i < textureCount; i++)
	{
		if (textures[i] == nullptr)
			continue;

		texturesImpl.at(i) = reinterpret_cast<TextureMetal*>(textures[i]);
	}

	if (depthTexture != nullptr)
	{
		depthTextureImpl = reinterpret_cast<TextureMetal*>(depthTexture);
	}

	TextureMetal* resolvedTextureImpl = nullptr;
	TextureMetal* resolvedDepthTextureImpl = nullptr;

	if (resolvedTexture != nullptr)
	{
		resolvedTextureImpl = reinterpret_cast<TextureMetal*>(resolvedTexture);
	}

	if (resolvedDepthTexture != nullptr)
	{
		resolvedDepthTextureImpl = reinterpret_cast<TextureMetal*>(resolvedDepthTexture);
	}

	UpdateTarget(texturesImpl.data(), textureCount, depthTextureImpl, resolvedTextureImpl, resolvedDepthTextureImpl);

	return true;
}

void RenderPassMetal::SetIsColorCleared(bool isColorCleared)
{
	this->isColorCleared = isColorCleared;
	RenderPass::SetIsColorCleared(isColorCleared);
}

void RenderPassMetal::SetIsDepthCleared(bool isDepthCleared)
{
	this->isDepthCleared = isDepthCleared;
	RenderPass::SetIsDepthCleared(isDepthCleared);
}

void RenderPassMetal::SetClearColor(const Color8& color)
{
	this->clearColor = color;
	RenderPass::SetClearColor(color);
}

RenderPassPipelineStateMetal::RenderPassPipelineStateMetal() {}

void RenderPassPipelineStateMetal::SetKey(const RenderPassPipelineStateKey& key)
{
	Key = key;
    pixelFormats.resize(key.RenderTargetFormats.size());

    for (size_t i = 0; i < pixelFormats.size(); i++)
    {
        pixelFormats.at(i) = ConvertFormat(key.RenderTargetFormats.at(i));
    }

    depthStencilFormat = ConvertFormat(key.DepthFormat);
}

}
