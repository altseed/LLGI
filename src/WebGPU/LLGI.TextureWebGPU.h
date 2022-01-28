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

public:
	bool Initialize(wgpu::Device& device, const TextureParameter& parameter);
};

} // namespace LLGI