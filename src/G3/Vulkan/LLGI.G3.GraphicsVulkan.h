#pragma once

#include "../LLGI.G3.Graphics.h"

namespace LLGI
{
namespace G3
{

class RenderPassVulkan : public RenderPass
{
};

class TempMemoryPool
{
public:
};

class GraphicsVulkan : public Graphics
{
private:
	int32_t swapBufferCount_ = 0;

public:
	GraphicsVulkan(int32_t swapBufferCount);

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
};

} // namespace G3
} // namespace LLGI