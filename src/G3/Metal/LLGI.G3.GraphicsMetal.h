#pragma once

#include "../LLGI.G3.Graphics.h"
#import <MetalKit/MetalKit.h>
#include <functional>

namespace LLGI
{
namespace G3
{

struct Graphics_Impl;
struct RenderPass_Impl;

class GraphicsMetal;
class RenderPassMetal;
    
class RenderPassMetal : public RenderPass
{
    GraphicsMetal* graphics_ = nullptr;
    bool isStrongRef_ = false;
	RenderPass_Impl* impl = nullptr;

public:
    RenderPassMetal(GraphicsMetal* graphics, bool isStrongRef);

    virtual ~RenderPassMetal();

    void SetIsColorCleared(bool isColorCleared) override;
    
    void SetIsDepthCleared(bool isDepthCleared) override;
    
    void SetClearColor(const Color8& color) override;
    
	RenderPass_Impl* GetImpl() const;
};

struct GraphicsView
{
    id<CAMetalDrawable> drawable;
};
    
class GraphicsMetal : public Graphics
{
	Graphics_Impl* impl = nullptr;
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

	Graphics_Impl* GetImpl() const;
};

} // namespace G3
} // namespace LLGI
