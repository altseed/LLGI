#pragma once

#include "../LLGI.G3.Graphics.h"

namespace LLGI
{
namespace G3
{

struct Graphics_Impl;
struct RenderPass_Impl;

class RenderPassMetal : public RenderPass
{
	RenderPass_Impl* impl = nullptr;

public:
	RenderPassMetal() = default;

	virtual ~RenderPassMetal() = default;

	RenderPass_Impl* GetImpl() const;
};

class GraphicsMetal : public Graphics
{
	Graphics_Impl* impl = nullptr;

public:
	GraphicsMetal();
	virtual ~GraphicsMetal();

	bool Initialize();

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

	ConstantBuffer* CreateConstantBuffer(int32_t size, ConstantBufferType type) override;

	RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) override;

	Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) override;

	Texture* CreateTexture(uint64_t id) override;

	Graphics_Impl* GetImpl() const;
};

} // namespace G3
} // namespace LLGI
