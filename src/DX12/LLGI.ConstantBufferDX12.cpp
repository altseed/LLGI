
#include "LLGI.ConstantBufferDX12.h"
#include "../LLGI.ConstantBuffer.h"

namespace LLGI
{

ConstantBufferDX12::ConstantBufferDX12() {}

bool ConstantBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// resourceDesc.Width = size * sizeof(Constant3D);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	auto hr = graphics_->GetDevice()->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	SafeAddRef(constantBuffer);
	return true;

FAILED_EXIT:
	SafeRelease(constantBuffer);
	return false;
}

void* ConstantBufferDX12::Lock()
{

FAILED_EXIT:
	return nullptr;
}

void* ConstantBufferDX12::Lock(int32_t offset, int32_t size)
{

FAILED_EXIT:
	return nullptr;
}

void ConstantBufferDX12::Unlock() { constantBuffer->Unmap(0, nullptr); }

int32_t ConstantBufferDX12::GetSize() { return sizeof(float) * 9; }

} // namespace LLGI
