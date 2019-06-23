#include "LLGI.DescriptorHeapDX12.h"

namespace LLGI
{

DescriptorHeapDX12::DescriptorHeapDX12(std::shared_ptr<GraphicsDX12> graphics, int size, int stage)
	: graphics_(graphics), size_(size), stage_(stage)
{
	for (int i = 0; i < numHeaps_; i++)
	{
		auto heap = CreateHeap(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i), size_ * stage_);
		assert(heap != nullptr);
		descriptorHeaps_[i] = heap;
		cpuHandles_[i] = descriptorHeaps_[i]->GetCPUDescriptorHandleForHeapStart();
		gpuHandles_[i] = descriptorHeaps_[i]->GetGPUDescriptorHandleForHeapStart();
	}
}

DescriptorHeapDX12::~DescriptorHeapDX12()
{
	SafeRelease(graphics_);
	for (int i = 0; i < numHeaps_; i++)
		SafeRelease(descriptorHeaps_[i]);
}

void DescriptorHeapDX12::IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int count)
{
	auto size = graphics_->GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	auto i = static_cast<int>(heapType);
	cpuHandles_[i].ptr += size * count;
}

void DescriptorHeapDX12::IncrementGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int count)
{
	auto size = graphics_->GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	auto i = static_cast<int>(heapType);
	gpuHandles_[i].ptr += size * count;
}

ID3D12DescriptorHeap* DescriptorHeapDX12::GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return descriptorHeaps_[static_cast<int>(heapType)];
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapDX12::GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return cpuHandles_[static_cast<int>(heapType)];
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapDX12::GetGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return gpuHandles_[static_cast<int>(heapType)];
}

void DescriptorHeapDX12::Reset()
{
	for (int i = 0; i < numHeaps_; i++)
	{
		cpuHandles_[i] = descriptorHeaps_[i]->GetCPUDescriptorHandleForHeapStart();
		gpuHandles_[i] = descriptorHeaps_[i]->GetGPUDescriptorHandleForHeapStart();
	}
}

ID3D12DescriptorHeap* DescriptorHeapDX12::CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12DescriptorHeap* heap = nullptr;

	heapDesc.NumDescriptors = numDescriptors;
	heapDesc.Type = heapType;
	heapDesc.Flags = (heapType <= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
						 ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
						 : D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Only descriptor heaps for CBV, SRV, UAV, sampler can be shader visible.

	heapDesc.NodeMask = 1; // TODO: set properly for multi-adaptor.

	auto hr = graphics_->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));
	if (FAILED(hr))
	{
		SafeRelease(heap);
		return nullptr;
	}
	return heap;
}

} // namespace LLGI
