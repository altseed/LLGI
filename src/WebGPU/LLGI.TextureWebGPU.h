#pragma once

#include "../LLGI.Graphics.h"
#include "../LLGI.Texture.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class TextureWebGPU : public Texture
{
	wgpu::Texture texture_;
	wgpu::TextureView textureView_;
	TextureParameter parameter_;

public:
	bool Initialize(wgpu::Device& device, const TextureParameter& parameter);

	const TextureParameter& GetParameter() const { return parameter_; }

	wgpu::Texture GetTexture() const { return texture_; }
};

} // namespace LLGI