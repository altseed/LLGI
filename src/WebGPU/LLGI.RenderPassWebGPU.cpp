#include "LLGI.RenderPassWebGPU.h"
#include "LLGI.TextureWebGPU.h"

namespace LLGI
{

bool RenderPassWebGPU::Initialize(
	Texture** textures, int textureCount, Texture* depthTexture, Texture* resolvedRenderTexture, Texture* resolvedDepthTexture)
{
	if (!assignRenderTextures(textures, textureCount))
	{
		return false;
	}

	if (!assignDepthTexture(depthTexture))
	{
		return false;
	}

	if (!assignResolvedRenderTexture(resolvedRenderTexture))
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

	std::array<TextureWebGPU*, RenderTargetMax> texturesImpl;
	texturesImpl.fill(nullptr);
	TextureWebGPU* depthTextureImpl = nullptr;

	for (int32_t i = 0; i < textureCount; i++)
	{
		if (textures[i] == nullptr)
			continue;

		texturesImpl.at(i) = reinterpret_cast<TextureWebGPU*>(textures[i]);
	}

	if (depthTexture != nullptr)
	{
		depthTextureImpl = reinterpret_cast<TextureWebGPU*>(depthTexture);
	}

	TextureWebGPU* resolvedTextureImpl = nullptr;
	TextureWebGPU* resolvedDepthTextureImpl = nullptr;

	if (resolvedRenderTexture != nullptr)
	{
		resolvedTextureImpl = reinterpret_cast<TextureWebGPU*>(resolvedRenderTexture);
	}

	if (resolvedDepthTexture != nullptr)
	{
		resolvedDepthTextureImpl = reinterpret_cast<TextureWebGPU*>(resolvedDepthTexture);
	}

	std::array<wgpu::RenderPassColorAttachment, RenderTargetMax> colorAttachments;

	descriptor_.colorAttachmentCount = textureCount;
	descriptor_.colorAttachments = colorAttachments.data();

	for (int i = 0; i < textureCount; i++)
	{
		colorAttachments[i].view = texturesImpl[i]->GetTextureView();

		if(GetIsColorCleared())
		{
			// TODO
		}
		else
		{
			// TODO
		}

		if (resolvedTextureImpl != nullptr)
		{
			colorAttachments[i].resolveTarget = resolvedDepthTextureImpl->GetTextureView();
			colorAttachments[i].storeOp = wgpu::StoreOp::Store;
		}
	}

	wgpu::RenderPassDepthStencilAttachment depthStencilAttachiment;

	if (depthTexture != nullptr)
	{
		depthStencilAttachiment.view = depthTextureImpl->GetTextureView();

		if(GetIsDepthCleared())
		{
			// TODO
		}
		else
		{
			// TODO
		}

		if (resolvedDepthTextureImpl != nullptr)
		{
			// ?
		}
	}

	throw "Not implemented";
}

} // namespace LLGI