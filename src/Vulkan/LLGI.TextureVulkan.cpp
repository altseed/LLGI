
#include "LLGI.TextureVulkan.h"

namespace LLGI
{

TextureVulkan::TextureVulkan() {}

TextureVulkan::~TextureVulkan()
{
	if (image != nullptr)
	{
		graphics_->GetDevice().destroyImageView(view);
		graphics_->GetDevice().destroyImage(image);
		graphics_->GetDevice().freeMemory(devMem);

		image = nullptr;
		view = nullptr;
	}

	SafeRelease(graphics_);
}

bool TextureVulkan::Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	// TODO many things

	vk::ImageCreateInfo createInfo;

	createInfo.imageType = vk::ImageType::e2D;
	createInfo.extent.width = size.X;
	createInfo.extent.height = size.Y;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = vk::Format::eR8G8B8A8Unorm;
	createInfo.tiling = vk::ImageTiling::eOptimal;

	textureSize = size;

	return false;
}

void* TextureVulkan::Lock() { throw "Not inplemented"; }

void TextureVulkan::Unlock() { throw "Not inplemented"; }

Vec2I TextureVulkan::GetSizeAs2D() { return textureSize; }

bool TextureVulkan::IsRenderTexture() const { throw "Not inplemented"; }

bool TextureVulkan::IsDepthTexture() const { throw "Not inplemented"; }

} // namespace LLGI
