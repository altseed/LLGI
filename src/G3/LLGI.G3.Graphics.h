
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class RenderTarget
	: public ReferenceObject
{
public:
	RenderTarget() = default;
	virtual ~RenderTarget() = default;
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
		@brief get render target of screen to show on a display.
		@note
		Don't release and addref it.
		Don't use it for the many purposes, please input Clear or SetRenderTarget immediately.
	*/
	virtual RenderTarget* GetCurrentScreen();
	virtual VertexBuffer* CreateVertexBuffer(int32_t size);
	virtual IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count);
	virtual Shader* CreateShader(DataStructure* data, int32_t count);
	virtual PipelineState* CreatePiplineState();
	virtual CommandList* CreateCommandList();
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size);
	virtual Texture* CreateTexture();
};

}
}