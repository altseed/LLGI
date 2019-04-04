
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class VertexBuffer;
class IndexBuffer;

class CommandList : public ReferenceObject
{
protected:
	struct BindingVertexBuffer
	{
		VertexBuffer* vertexBuffer = nullptr;
		int32_t stride = 0;
		int32_t offset = 0;
	};

private:

	BindingVertexBuffer bindingVertexBuffer;
	IndexBuffer* currentIndexBuffer = nullptr;
	PipelineState* currentPipelineState = nullptr;

	bool isVertexBufferDirtied = true;
	bool isCurrentIndexBufferDirtied = true;

protected:
	void GetCurrentVertexBuffer(BindingVertexBuffer& buffer, bool& isDirtied);
	void GetCurrentIndexBuffer(IndexBuffer*& buffer, bool& isDirtied); 
	void GetCurrentPipelineState(PipelineState*& pipelineState);

public:
	CommandList() = default;
	virtual ~CommandList() = default;

	virtual void Begin();
	virtual void End();

	virtual void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
	virtual void Draw(int32_t pritimiveCount);
	virtual void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset);
	virtual void SetIndexBuffer(IndexBuffer* indexBuffer);
	virtual void SetPipelineState(PipelineState* pipelineState);
	virtual void SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage);
	virtual void
	SetTexture(Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage);
	virtual void BeginRenderPass(RenderPass* renderPass) {}
	virtual void EndRenderPass() {}
};

} // namespace LLGI