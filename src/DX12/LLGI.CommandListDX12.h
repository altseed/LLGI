
#pragma once

#include "../LLGI.CommandList.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.DescriptorHeapDX12.h"
#include "LLGI.GraphicsDX12.h"
#include "LLGI.PipelineStateDX12.h"

namespace LLGI
{

class CommandListDX12 : public CommandList
{
private:
	struct SwapBuffer
	{
		std::shared_ptr<DescriptorHeapDX12> cbreDescriptorHeap;
		std::shared_ptr<DescriptorHeapDX12> rtDescriptorHeap;
		std::shared_ptr<DescriptorHeapDX12> smpDescriptorHeap;
		std::shared_ptr<ID3D12GraphicsCommandList> commandList;
		std::shared_ptr<ID3D12CommandAllocator> commandAllocator;
	};

	std::shared_ptr<GraphicsDX12> graphics_;
	std::shared_ptr<RenderPassDX12> renderPass_;

	std::vector<SwapBuffer> swapBuffers_;

public:
	CommandListDX12();
	virtual ~CommandListDX12();
	bool Initialize(GraphicsDX12* graphics);

	void Begin() override;
	void End() override;
	void BeginRenderPass(RenderPass* renderPass) override;
	void EndRenderPass() override;
	void Draw(int32_t pritimiveCount) override;

	void Clear(const Color8& color);

	ID3D12GraphicsCommandList* GetCommandList() const;
};

} // namespace LLGI
