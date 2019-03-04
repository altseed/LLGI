#pragma once

#include "../LLGI.G3.CommandList.h"
#include "LLGI.G3.BaseVulkan.h"

namespace LLGI
{
namespace G3
{

class CommandListVulkan : public CommandList
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::vector<vk::CommandBuffer> commandBuffers;

public:
	CommandListVulkan();
	virtual ~CommandListVulkan();

	bool Initialize(GraphicsVulkan* graphics);

	void Begin() override;
	void End() override;

	void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;
	void Draw(int32_t pritimiveCount) override;
	void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset) override;
	void SetIndexBuffer(IndexBuffer* indexBuffer) override;
	void SetPipelineState(PipelineState* pipelineState) override;
	void SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage) override;
	void SetTexture(
		Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage) override;
	void BeginRenderPass(RenderPass* renderPass) override;
	void EndRenderPass() override;
};

} // namespace G3
} // namespace LLGI