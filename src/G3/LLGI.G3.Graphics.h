
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class RenderPass
	: public ReferenceObject
{
public:
	RenderPass() = default;
	virtual ~RenderPass() = default;
};

class Graphics
	: public ReferenceObject
{
private:
public:
	Graphics() = default;
	virtual ~Graphics() = default;

	virtual void Execute(CommandList* commandList);

	/**
		@brief get render pass of screen to show on a display.
		@note
		Don't release and addref it.
		Don't use it for the many purposes, please input Clear or SetRenderPass immediately.
	*/
	virtual RenderPass* GetCurrentScreen();
	virtual VertexBuffer* CreateVertexBuffer(int32_t size);
	virtual IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count);
	virtual Shader* CreateShader(DataStructure* data, int32_t count);
	virtual PipelineState* CreatePiplineState();
	virtual CommandList* CreateCommandList();
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size);
	virtual RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) { return nullptr; }
	virtual Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer);
};

}
}