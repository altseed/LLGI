
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class VertexBuffer;
class IndexBuffer;

class CommandList
	: public ReferenceObject
{
private:
public:
	CommandList() = default;
	virtual ~CommandList() = default;

	virtual void Begin();
	virtual void End();

	/**
		@brief Clear renderTarget
		@note
		This function is composed of SetRenderTarget and Clear on seviral environment.
	*/
	virtual void Clear(RenderTarget* renderTarget, const Color8& color);

	virtual void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
	virtual void Draw(int32_t pritimiveCount);
	virtual void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride);
	virtual void SetIndexBuffer(IndexBuffer* indexBuffer);
	virtual void SetPipelineState(PipelineState* pipelineState);
	virtual void SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage);
	virtual void SetTexture(Texture* texture, int32_t unit, ShaderStageType shaderStage);
};

}
}