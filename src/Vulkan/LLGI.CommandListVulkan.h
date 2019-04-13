#pragma once

#include "../LLGI.CommandList.h"
#include "LLGI.BaseVulkan.h"

namespace LLGI
{

class CommandListVulkan : public CommandList
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::vector<vk::CommandBuffer> commandBuffers;
	std::array<ConstantBuffer*, static_cast<int>(ShaderStageType::Max)> constantBuffers;


	vk::DescriptorPool descriptorPool = nullptr;

public:
	CommandListVulkan();
	virtual ~CommandListVulkan();

	bool Initialize(GraphicsVulkan* graphics);

	void Begin() override;
	void End() override;

	void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;
	void Draw(int32_t pritimiveCount) override;
	void SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage) override;
	void SetTexture(
		Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage) override;
	void BeginRenderPass(RenderPass* renderPass) override;
	void EndRenderPass() override;
	vk::CommandBuffer GetCommandBuffer() const;
};

} // namespace LLGI