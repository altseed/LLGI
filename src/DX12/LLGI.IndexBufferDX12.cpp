
#include "LLGI.IndexBufferDX12.h"
#include "../LLGI.IndexBuffer.h"

namespace LLGI
{

IndexBufferDX12::IndexBufferDX12() {}

IndexBufferDX12::~IndexBufferDX12() { SafeRelease(indexBuffer); }

bool IndexBufferDX12::Initialize(GraphicsDX12* graphics, int32_t stride, int32_t count)
{
	D3D12_HEAP_PROPERTIES heapProperties = {};
	D3D12_RESOURCE_DESC resourceDesc = {};

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = stride * count;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);
	this->stride_ = stride;
	this->count_ = count;

	auto hr = graphics_->GetDevice()->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	return true;

FAILED_EXIT:
	SafeRelease(indexBuffer);
	return false;
}

void* IndexBufferDX12::Lock()
{
	auto hr = indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	return mapped;

FAILED_EXIT:
	return nullptr;
}

void* IndexBufferDX12::Lock(int32_t offset, int32_t size)
{
	auto hr = indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	return mapped + offset;

FAILED_EXIT:
	return nullptr;
}

void IndexBufferDX12::Unlock()
{
	indexBuffer->Unmap(0, nullptr);
	mapped = nullptr;
}

} // namespace LLGI
