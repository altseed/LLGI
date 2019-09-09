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
class TextureMetal;

class RenderPassMetal : public RenderPass
{
	GraphicsMetal* graphics_ = nullptr;
	bool isStrongRef_ = false;
	RenderPass_Impl* impl = nullptr;
	std::shared_ptr<RenderPassPipelineStateMetal> renderPassPipelineState;
	std::array<std::shared_ptr<TextureMetal>, 4> colorBuffers_ = {};

public:
	RenderPassMetal(GraphicsMetal* graphics, bool isStrongRef);

	virtual ~RenderPassMetal();

	void SetIsColorCleared(bool isColorCleared) override;

	void SetIsDepthCleared(bool isDepthCleared) override;

	void SetClearColor(const Color8& color) override;
	
	Texture* GetColorBuffer(int index) override;
	
	RenderPass_Impl* GetImpl() const;

	RenderPassPipelineState* CreateRenderPassPipelineState() override;

	void UpdateTarget(GraphicsMetal* graphics);
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

class SingleFrameMemoryPoolMetal : public SingleFrameMemoryPool
{
public:
	SingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount) {}
	virtual ~SingleFrameMemoryPoolMetal() = default;
	virtual void NewFrame() override { printf("Warning: Not implemented.¥n"); }
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size) override { printf("Warning: Not implemented.¥n"); }
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

	void SetWindowSize(const Vec2I& windowSize) override;

	void Execute(CommandList* commandList) override;

	void WaitFinish() override;

	RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;

	VertexBuffer* CreateVertexBuffer(int32_t size) override;

	IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count) override;

	Shader* CreateShader(DataStructure* data, int32_t count) override;

	PipelineState* CreatePiplineState() override;

	SingleFrameMemoryPool* CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount) override;

	CommandList* CreateCommandList(SingleFrameMemoryPool* memoryPool) override;

	ConstantBuffer* CreateConstantBuffer(int32_t size) override;

	RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) override;

	Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) override;

	Texture* CreateTexture(uint64_t id) override;

	std::shared_ptr<RenderPassPipelineStateMetal> CreateRenderPassPipelineState(MTLPixelFormat format);

	std::vector<uint8_t> CaptureRenderTarget(Texture* renderTarget) override;

	Graphics_Impl* GetImpl() const;
};

} // namespace LLGI
