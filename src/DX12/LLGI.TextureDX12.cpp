#include "LLGI.TextureDX12.h"

namespace LLGI
{
TextureDX12::TextureDX12(GraphicsDX12* graphics) : graphics_(graphics) { SafeAddRef(graphics_); }

TextureDX12::~TextureDX12() {}

bool TextureDX12::Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	if (isRenderPass)
		throw "Not implemented";

	if (isDepthBuffer)
		throw "Not implemented";

	D3D12_HEAP_PROPERTIES heapProperties = {};
	D3D12_RESOURCE_DESC resourceDesc = {};

	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = size.X;
	resourceDesc.Height = size.Y;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	auto hr = graphics_->GetDevice()->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texture_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	textureSize = size;

	return true;

FAILED_EXIT:
	SafeRelease(texture_);
	return false;
}

void* TextureDX12::Lock()
{
	auto hr = texture_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	return mapped;

FAILED_EXIT:
	return nullptr;
}

void TextureDX12::Unlock()
{
	texture_->Unmap(0, nullptr);
	mapped = nullptr;
}

Vec2I TextureDX12::GetSizeAs2D() { return textureSize; }

bool TextureDX12::IsRenderTexture() const
{
	throw "Not inplemented";
	return isRenderPass_;
}

bool TextureDX12::IsDepthTexture() const
{
	throw "Not inplemented";
	return isDepthBuffer_;
}

} // namespace LLGI