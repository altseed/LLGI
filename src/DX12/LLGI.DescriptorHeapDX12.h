#pragma once

#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

namespace LLGI
{
class DescriptorHeapDX12
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;
	int size_ = 0;
	int stage_ = 0;
	int offset_ = 0;

	std::array<ID3D12DescriptorHeap*, 2> descriptorHeaps_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> CpuHandles_;
	std::array<D3D12_GPU_DESCRIPTOR_HANDLE, 2> GpuHandles_;

	ID3D12DescriptorHeap* CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors);

public:
	DescriptorHeapDX12(std::shared_ptr<GraphicsDX12> graphics, int size, int stage);
	virtual ~DescriptorHeapDX12();

	void IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int count);
	void IncrementGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int count);
	ID3D12DescriptorHeap* GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	void Reset();
};

} // namespace LLGI
