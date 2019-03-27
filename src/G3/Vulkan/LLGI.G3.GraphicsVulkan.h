#pragma once

#include "../LLGI.G3.Graphics.h"
#include "LLGI.G3.BaseVulkan.h"
#include <functional>

namespace LLGI
{
namespace G3
{

class RenderPassVulkan : public RenderPass
{
private:
	GraphicsVulkan* graphics_ = nullptr;
	bool isStrongRef_ = false;
	Vec2I imageSize_;

public:
	vk::RenderPass renderPass;

	vk::Framebuffer frameBuffer;

	std::array<vk::Image, 4> colorBuffers;
	vk::Image depthBuffer;

	RenderPassVulkan(GraphicsVulkan* graphics, bool isStrongRef);
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

	Vec2I GetImageSize() const;
};

class TempMemoryPool
{
public:
};

class PlatformView
{
public:
	std::vector<vk::Image> colors;
	std::vector<vk::Image> depths;
	std::vector<vk::ImageView> colorViews;
	std::vector<vk::ImageView> depthViews;
	Vec2I imageSize;
	vk::Format format;
};

class PlatformStatus
{
public:
	int currentSwapBufferIndex;
};

class GraphicsVulkan : public Graphics
{
private:
	int32_t swapBufferCount_ = 0;
	int32_t currentSwapBufferIndex = -1;
	std::vector<std::shared_ptr<RenderPassVulkan>> renderPasses;
	vk::Image currentColorBuffer;

	vk::Device vkDevice;
	vk::Queue vkQueue;
	vk::CommandPool vkCmdPool;
	vk::PhysicalDevice vkPysicalDevice;

	std::function<void(PlatformStatus&)> getStatus_;

public:
	GraphicsVulkan(const vk::Device& device,
				   const vk::Queue& quque,
				   const vk::CommandPool& commandPool,
				   const vk::PhysicalDevice& pysicalDevice,
				   const PlatformView& platformView,
				   std::function<void(PlatformStatus&)> getStatus);

	virtual ~GraphicsVulkan();

	void NewFrame() override;

	void SetWindowSize(const Vec2I& windowSize) override;

	void Execute(CommandList* commandList) override;

	void WaitFinish() override;

	RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;
	VertexBuffer* CreateVertexBuffer(int32_t size) override;
	IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count) override;
	Shader* CreateShader(DataStructure* data, int32_t count) override;
	PipelineState* CreatePiplineState() override;
	CommandList* CreateCommandList() override;
	ConstantBuffer* CreateConstantBuffer(int32_t size, ConstantBufferType type = ConstantBufferType::LongTime) override;
	RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) override;
	Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) override;
	Texture* CreateTexture(uint64_t id) override;

	vk::Device GetDevice() const { return vkDevice; }
	vk::CommandPool GetCommandPool() const { return vkCmdPool; }
	vk::Queue GetQueue() const { return vkQueue; }

	int32_t GetCurrentSwapBufferIndex() const;
	int32_t GetSwapBufferCount() const;
	uint32_t GetMemoryTypeIndex(uint32_t bits, const vk::MemoryPropertyFlags& properties);
};

} // namespace G3
} // namespace LLGI