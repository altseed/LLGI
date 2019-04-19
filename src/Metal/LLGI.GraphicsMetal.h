#pragma once

#include "../LLGI.Graphics.h"
#import <MetalKit/MetalKit.h>
#include <functional>
#include <unordered_map>

namespace LLGI
{

struct Graphics_Impl;
struct RenderPass_Impl;
struct RenderPassPipelineState_Impl;

class GraphicsMetal;
class RenderPassMetal;
class RenderPassPipelineStateMetal;

class RenderPassMetal : public RenderPass
{
	GraphicsMetal* graphics_ = nullptr;
	bool isStrongRef_ = false;
	RenderPass_Impl* impl = nullptr;
	std::shared_ptr<RenderPassPipelineStateMetal> renderPassPipelineState;

public:
	RenderPassMetal(GraphicsMetal* graphics, bool isStrongRef);

	virtual ~RenderPassMetal();

	void SetIsColorCleared(bool isColorCleared) override;

	void SetIsDepthCleared(bool isDepthCleared) override;

	void SetClearColor(const Color8& color) override;

	RenderPass_Impl* GetImpl() const;

	RenderPassPipelineState* CreateRenderPassPipelineState() override;
};

class RenderPassPipelineStateMetal : public RenderPassPipelineState
{
private:
	RenderPassPipelineState_Impl* impl = nullptr;

public:
	RenderPassPipelineStateMetal();
	virtual ~RenderPassPipelineStateMetal();

	RenderPassPipelineState_Impl* GetImpl() const;
};

struct RenderPassPipelineStateMetalKey
{
	MTLPixelFormat format;

	bool operator==(const RenderPassPipelineStateMetalKey& value) const { return (format == value.format); }

	struct Hash
	{
		typedef std::size_t result_type;

		std::size_t operator()(const RenderPassPipelineStateMetalKey& key) const
		{
			return std::hash<std::int32_t>()(static_cast<int>(key.format));
		}
	};
};

struct GraphicsView
{
	id<CAMetalDrawable> drawable;
};

class GraphicsMetal : public Graphics
{
	Graphics_Impl* impl = nullptr;

	std::unordered_map<RenderPassPipelineStateMetalKey, std::weak_ptr<RenderPassPipelineStateMetal>, RenderPassPipelineStateMetalKey::Hash>
		renderPassPipelineStates;

	std::shared_ptr<RenderPassMetal> renderPass_ = nullptr;
	std::function<GraphicsView()> getGraphicsView_;

public:
	GraphicsMetal();
	virtual ~GraphicsMetal();

	bool Initialize(std::function<GraphicsView()> getGraphicsView);

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

	std::shared_ptr<RenderPassPipelineStateMetal> CreateRenderPassPipelineState(MTLPixelFormat format);

	Graphics_Impl* GetImpl() const;
};

} // namespace LLGI
