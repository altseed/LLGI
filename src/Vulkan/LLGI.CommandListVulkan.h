#pragma once

#include "../LLGI.CommandList.h"
#include "LLGI.BaseVulkan.h"

namespace LLGI
{

class ShortTimeContaintBufferCache
{
};

class DescriptorPoolVulkan
{
private:
	GraphicsVulkan* graphics_;
	vk::DescriptorPool descriptorPool = nullptr;
	int32_t size_ = 0;
	int32_t stage_ = 0;
	int32_t offset = 0;
	std::vector<std::vector<vk::DescriptorSet>> cache;

public:
	DescriptorPoolVulkan(GraphicsVulkan* graphics, int32_t size, int stage);
	virtual ~DescriptorPoolVulkan();
	const std::vector<vk::DescriptorSet>& Get(PipelineStateVulkan* pip);
	void Reset();
};

// コマンドリスト一つ分。また、それの実行中に保持なければならないデータを管理する。
class VulkanNativeCommandList
{
private:
    GraphicsVulkan* graphics_ = nullptr;
	VkCommandBuffer nativeCommandBuffer_;
	std::shared_ptr<DescriptorPoolVulkan> descriptorPool_;
	std::unique_ptr<ShortTimeContaintBufferCache> constantBufferCache_;

public:
	bool Initialize(GraphicsVulkan* graphics);
	void Dispose();

    bool Begin();
    bool End();

    VkCommandBuffer GetNativeCommandBuffer() const { return nativeCommandBuffer_; }
    DescriptorPoolVulkan* GetDescriptorPool() const { return descriptorPool_.get(); }
};

class CommandListVulkan : public CommandList
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::vector<std::shared_ptr<VulkanNativeCommandList>> commandLists_;
	// std::vector<vk::CommandBuffer> commandBuffers;
	// std::vector<std::shared_ptr<DescriptorPoolVulkan>> descriptorPools;

public:
	CommandListVulkan();
	virtual ~CommandListVulkan();

	bool Initialize(GraphicsVulkan* graphics);

	void Begin() override;
	void End() override;

	void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height) override;
	void Draw(int32_t pritimiveCount) override;
	void BeginRenderPass(RenderPass* renderPass) override;
	void EndRenderPass() override;
    VkCommandBuffer GetCommandBuffer() const;
};

} // namespace LLGI