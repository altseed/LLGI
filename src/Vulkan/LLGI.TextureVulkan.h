
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
	vk::Format vkTextureFormat;

	Vec2I textureSize;

	int32_t memorySize = 0;
	std::unique_ptr<Buffer> cpuBuf;
	void* data = nullptr;

	bool isRenderPass_ = false;
	bool isDepthBuffer_ = false;

public:
	TextureVulkan(GraphicsVulkan* graphics);
	virtual ~TextureVulkan();

	bool Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer);

	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;

	const vk::ImageView& GetView() const { return view; }

	vk::Format GetVulkanFormat() const { return vkTextureFormat; }
};

} // namespace LLGI
