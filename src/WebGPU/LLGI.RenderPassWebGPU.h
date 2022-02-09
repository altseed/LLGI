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
	bool
	Initialize(Texture** textures, int textureCount, Texture* depthTexture, Texture* resolvedRenderTexture, Texture* resolvedDepthTexture);

	const wgpu::RenderPassDescriptor& GetDescriptor() const { return descriptor_; }
};

} // namespace LLGI