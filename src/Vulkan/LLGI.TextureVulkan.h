
#pragma once

#include "../LLGI.Texture.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

class TextureVulkan : public Texture
{
private:
	GraphicsVulkan* graphics_ = nullptr;
	vk::Image image = nullptr;
	vk::ImageView view = nullptr;
	vk::DeviceMemory devMem = nullptr;
	Vec2I textureSize;

public:
	TextureVulkan();
	virtual ~TextureVulkan();

	bool Initialize(const Vec2I &size, bool isRenderPass, bool isDepthBuffer);

	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;
};

} // namespace LLGI
