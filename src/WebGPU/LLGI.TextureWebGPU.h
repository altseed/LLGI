#pragma once

#include "../LLGI.Graphics.h"
#include "../LLGI.Texture.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class TextureWebGPU : public Texture
{
	wgpu::Device device_;

	wgpu::Texture texture_;
	wgpu::TextureView textureView_;
	TextureParameter parameter_;
	std::vector<uint8_t> temp_buffer_;

public:
	bool Initialize(wgpu::Device& device, const TextureParameter& parameter);
	void* Lock() override;
	void Unlock() override;

	const TextureParameter& GetParameter() const { return parameter_; }

	wgpu::Texture GetTexture() const { return texture_; }

	wgpu::TextureView GetTextureView() const { return textureView_; }
};

} // namespace LLGI