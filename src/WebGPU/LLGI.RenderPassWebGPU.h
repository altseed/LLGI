#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{
class TextureWebGPU;

class RenderPassWebGPU : public RenderPass
{
	wgpu::RenderPassDescriptor descriptor_;

public:
	RenderPassWebGPU(TextureWebGPU** textures,
					 int numTextures,
					 TextureWebGPU* depthTexture,
					 TextureWebGPU* resolvedRenderTexture,
					 TextureWebGPU* resolvedDepthTexture);

	const wgpu::RenderPassDescriptor& GetDescriptor() const { return descriptor_; }
};

} // namespace LLGI