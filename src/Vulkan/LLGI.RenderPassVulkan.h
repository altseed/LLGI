#pragma once

#include "../LLGI.Graphics.h"
#include "../Utils/LLGI.FixedSizeVector.h"
#include "LLGI.BaseVulkan.h"
#include <functional>
#include <unordered_map>

namespace LLGI
{

class RenderPassVulkan;
class RenderPassPipelineStateVulkan;
class RenderPassPipelineStateCacheVulkan;
class TextureVulkan;

class RenderPassVulkan : public RenderPass
{
private:
	RenderPassPipelineStateCacheVulkan* renderPassPipelineStateCache_ = nullptr;
	vk::Device device_;
	ReferenceObject* owner_ = nullptr;

	Vec2I imageSize_;
	std::shared_ptr<TextureVulkan> depthBufferPtr;
	
public:
	struct RenderTargetProperty
	{
		vk::Format format;
		vk::Image colorBuffer;
		std::shared_ptr<TextureVulkan> colorBufferPtr;
	};

	RenderPassPipelineStateVulkan* renderPassPipelineState = nullptr;

	vk::Framebuffer frameBuffer_;

	bool isPresentMode = false;
	bool hasDepth = false;

	FixedSizeVector<RenderTargetProperty, RenderTargetMax> renderTargetProperties;

	vk::Image depthBuffer;

	RenderPassVulkan(RenderPassPipelineStateCacheVulkan* renderPassPipelineStateCache, vk::Device device, ReferenceObject* owner);
	virtual ~RenderPassVulkan();

	/**
		@brief	initialize for screen
	*/
	bool Initialize(const vk::Image& imageColor,
					const vk::Image& imageDepth,
					const vk::ImageView& imageColorView,
					const vk::ImageView& imageDepthView,
					Vec2I imageSize,
					vk::Format format);

	/**
		@brief	initialize for offscreen
	*/
	bool Initialize(const TextureVulkan** textures, int32_t textureCount, TextureVulkan* depthTexture);

	Vec2I GetImageSize() const;

	virtual Texture* GetColorBuffer(int index) override;
};

class RenderPassPipelineStateVulkan : public RenderPassPipelineState
{
private:
	vk::Device device_;
	ReferenceObject* owner_ = nullptr;

public:
	RenderPassPipelineStateVulkan(vk::Device device, ReferenceObject* owner);

	virtual ~RenderPassPipelineStateVulkan();

	vk::RenderPass renderPass_;

	vk::RenderPass GetRenderPass() const;
};

struct RenderPassPipelineStateVulkanKey
{
	bool isPresentMode;
	FixedSizeVector<vk::Format, RenderTargetMax> formats;
	bool hasDepth;

	bool operator==(const RenderPassPipelineStateVulkanKey& value) const
	{
		return (isPresentMode == value.isPresentMode && hasDepth == value.hasDepth && formats == value.formats);
	}

	struct Hash
	{
		typedef std::size_t result_type;

		std::size_t operator()(const RenderPassPipelineStateVulkanKey& key) const
		{
			return key.formats.get_hash() + std::hash<bool>()(key.isPresentMode) + std::hash<bool>()(key.hasDepth);
		}
	};
};

} // namespace LLGI

/*
template <> struct std::hash<size_t>
{
	_NODISCARD size_t operator()(const size_t& _Keyval) const noexcept { return std::hash<uint64_t>()(static_cast<uint64_t>(_Keyval)); }
};

template <> struct std::hash<vk::Format>
{
	_NODISCARD size_t operator()(const vk::Format& _Keyval) const noexcept { return std::hash<uint64_t>()(static_cast<uint64_t>(_Keyval)); }
};
*/