
#pragma once

#include "../LLGI.CommandList.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"
#include "LLGI.PipelineStateDX12.h"

namespace LLGI
{

class DescriptorHeapDX12
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;
	int size_ = 0;
	int stage_ = 0;
	int offset_ = 0;

	std::vector<ID3D12DescriptorHeap*> descriptorHeaps_;
	std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> cache_;

	ID3D12DescriptorHeap* CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors);

public:
	DescriptorHeapDX12(std::shared_ptr<GraphicsDX12> graphics, int size, int stage);
	virtual ~DescriptorHeapDX12();
	std::vector<ID3D12DescriptorHeap*>& GetHeaps();
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> GetCPUHandles();
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> GetGPUHandles();
	void Reset() { offset_ = 0; }
};

class CommandListDX12 : public CommandList
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;
	std::shared_ptr<RenderPassDX12> renderPass_;

	std::vector<std::shared_ptr<ID3D12GraphicsCommandList>> commandLists;
	std::vector<std::shared_ptr<ID3D12CommandAllocator>> commandAllocators;

	std::vector<std::shared_ptr<DescriptorHeapDX12>> descriptorHeaps_;

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
