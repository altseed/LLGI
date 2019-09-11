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

} // namespace LLGI
