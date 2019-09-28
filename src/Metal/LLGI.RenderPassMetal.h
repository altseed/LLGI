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
    RenderPass_Impl* impl = nullptr;
	std::array<std::shared_ptr<TextureMetal>, 8> colorBuffers_ = {};

public:
	RenderPassMetal();

	virtual ~RenderPassMetal();
    
    void UpdateRenderTarget(Texture** textures, int32_t textureCount, Texture* depthTexture);
    
	void SetIsColorCleared(bool isColorCleared) override;

	void SetIsDepthCleared(bool isDepthCleared) override;

	void SetClearColor(const Color8& color) override;
	
	Texture* GetColorBuffer(int index) override;
	
	RenderPass_Impl* GetImpl() const;
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

} // namespace LLGI
