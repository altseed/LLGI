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
public:
	std::array<vk::Image, 4> colorBuffers;
	vk::Image depthBuffer;
};

class TempMemoryPool
{
public:
};

class PlatformStatus
{
public:
	vk::Image colorBuffer;
	int currentSwapBufferIndex;
};

class GraphicsVulkan : public Graphics
{
private:
	int32_t swapBufferCount_ = 0;
	int32_t currentSwapBufferIndex = -1;
	std::shared_ptr<RenderPassVulkan> currentRenderPass = nullptr;
	vk::Image currentColorBuffer;

	vk::Device vkDevice;
	vk::CommandPool vkCmdPool;

	std::function<void(PlatformStatus&)> getStatus_;

public:
	GraphicsVulkan(const vk::Device& device,
				   const vk::CommandPool& commandPool,
				   int32_t swapBufferCount,
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

	int32_t GetCurrentSwapBufferIndex() const;
	int32_t GetSwapBufferCount() const;
};

} // namespace G3
} // namespace LLGI