
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{
static constexpr int NumTexture = 8;

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

	struct BindingTexture
	{
		Texture* texture = nullptr;
		TextureWrapMode wrapMode = TextureWrapMode::Clamp;
		TextureMinMagFilter minMagFilter = TextureMinMagFilter::Nearest;
	};

private:
	BindingVertexBuffer bindingVertexBuffer;
	IndexBuffer* currentIndexBuffer = nullptr;
	PipelineState* currentPipelineState = nullptr;

	bool isVertexBufferDirtied = true;
	bool isCurrentIndexBufferDirtied = true;
	bool isPipelineDirtied = true;

	std::array<ConstantBuffer*, static_cast<int>(ShaderStageType::Max)> constantBuffers;

protected:
	std::array<std::array<BindingTexture, NumTexture>, static_cast<int>(ShaderStageType::Max)> currentTextures;

protected:
	void GetCurrentVertexBuffer(BindingVertexBuffer& buffer, bool& isDirtied);
	void GetCurrentIndexBuffer(IndexBuffer*& buffer, bool& isDirtied);
	void GetCurrentPipelineState(PipelineState*& pipelineState, bool& isDirtied);
	void GetCurrentConstantBuffer(ShaderStageType type, ConstantBuffer*& buffer);

public:
	CommandList();
	virtual ~CommandList();

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
	virtual void BeginRenderPass(RenderPass* renderPass);
	virtual void EndRenderPass() {}
};

} // namespace LLGI
