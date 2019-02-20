
#include "LLGI.G3.IndexBufferDX12.h"
#include "../LLGI.G3.IndexBuffer.h"

namespace LLGI
{
namespace G3
{

IndexBufferDX12::IndexBufferDX12() {}

bool IndexBufferDX12::Initialize(GraphicsDX12* graphics, int32_t size)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// resourceDesc.Width = size * sizeof(Index3D);
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
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&IndexBuffer));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	SafeAddRef(indexBuffer);
	return true;

FAILED_EXIT:
	SafeRelease(indexBuffer);
	return false;
}

void* IndexBufferDX12::Lock()
{

FAILED_EXIT:
	return nullptr;
}

void* IndexBufferDX12::Lock(int32_t offset, int32_t size)
{

FAILED_EXIT:
	return nullptr;
}

void IndexBufferDX12::Unlock() { indexBuffer->Unmap(0, nullptr); }

} // namespace G3
} // namespace LLGI
