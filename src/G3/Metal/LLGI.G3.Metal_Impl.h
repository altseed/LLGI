#pragma once

#include "../LLGI.G3.Graphics.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
namespace G3
{

struct CommandList_Impl;
struct Buffer_Impl;

struct Graphics_Impl
{
	id<MTLDevice> device = nullptr;
	id<MTLCommandQueue> commandQueue = nullptr;
    id<CAMetalDrawable> drawable;
    
	Graphics_Impl();
	virtual ~Graphics_Impl();
	bool Initialize();
	void Execute(CommandList_Impl* commandBuffer);
};

struct RenderPass_Impl
{
	MTLRenderPassDescriptor* renderPassDescriptor;
	Color8 clearColor;
	bool isColorCleared;
	bool isDepthCleared;

    RenderPass_Impl();
    ~RenderPass_Impl();
	bool Initialize();
};

struct CommandList_Impl
{
	Graphics_Impl* graphics_ = nullptr;
	id<MTLCommandBuffer> commandBuffer = nullptr;
	id<MTLRenderCommandEncoder> renderEncoder = nullptr;

	CommandList_Impl();
	~CommandList_Impl();

	bool Initialize(Graphics_Impl* graphics);

	void Begin();
	void End();
	void BeginRenderPass(RenderPass_Impl* renderPass);
	void EndRenderPass();
	void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
	void SetVertexBuffer(Buffer_Impl* vertexBuffer, int32_t stride, int32_t offset);
};

struct Shader_Impl
{
    id<MTLLibrary> library = nullptr;
    
    Shader_Impl();
    ~Shader_Impl();
    bool Initialize(Graphics_Impl* graphics, void* data, int size);
};

struct PipelineState_Impl
{
	id<MTLRenderPipelineState> pipelineState;

	void Compile(Graphics_Impl* graphics);
};

struct Buffer_Impl
{
	id<MTLBuffer> buffer = nullptr;
	int32_t size_;

	Buffer_Impl();
	virtual ~Buffer_Impl();

	bool Initialize(Graphics_Impl* graphics, int32_t size);

	void* GetBuffer();
};

struct Texture_Impl
{
	id<MTLTexture> texture;

	bool Initialize(Graphics_Impl* graphics, const Vec2I& size);
};

} // namespace G3
} // namespace LLGI
