#include "LLGI.TextureDX12.h"
#include "LLGI.CommandListDX12.h"

namespace LLGI
{
TextureDX12::TextureDX12(GraphicsDX12* graphics) : graphics_(graphics) { SafeAddRef(graphics_); }

TextureDX12::~TextureDX12()
{
	SafeRelease(graphics_);
	SafeRelease(texture_);
	SafeRelease(buffer_);
	SafeRelease(SrvDescriptorHeap);
	SafeRelease(samplerDescriptorHeap);
}

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
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto hr = graphics_->GetDevice()->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	textureSize = size;

	// TODO: when it's NOT editable, do NOT call CreateBuffer.
	CreateBuffer();

	return true;

FAILED_EXIT:
	SafeRelease(texture_);
	return false;
}

void TextureDX12::CreateView()
{
	if (SrvDescriptorHeap != nullptr)
		SafeRelease(SrvDescriptorHeap);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	auto hr = graphics_->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&SrvDescriptorHeap));
	if (FAILED(hr))
	{
		goto FAIL_EXIT;
	}

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	auto handle = SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	graphics_->GetDevice()->CreateShaderResourceView(texture_, &srvDesc, handle);

	return;
FAIL_EXIT:
	SafeRelease(SrvDescriptorHeap);
}

void TextureDX12::CreateBuffer()
{
	UINT64 size = 0;
	graphics_->GetDevice()->GetCopyableFootprints(&texture_->GetDesc(), 0, 1, 0, &footprint, nullptr, nullptr, &size);

	D3D12_HEAP_PROPERTIES heapProperties = {};
	D3D12_RESOURCE_DESC resourceDesc = {};

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto hr = graphics_->GetDevice()->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer_));

	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}
	return;

FAILED_EXIT:
	SafeRelease(buffer_);
}

void TextureDX12::CreateSampler(TextureWrapMode wrapMode)
{
	if (samplerDescriptorHeap != nullptr)
		SafeRelease(samplerDescriptorHeap);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	D3D12_SAMPLER_DESC samplerDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE handle;

	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	auto hr = graphics_->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&samplerDescriptorHeap));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	// TODO
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	if (wrapMode == TextureWrapMode::Repeat)
	{
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
	else
	{
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	}
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

	handle = samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	graphics_->GetDevice()->CreateSampler(&samplerDesc, handle);
	return;
FAILED_EXIT:
	SafeRelease(samplerDescriptorHeap);
}

void* TextureDX12::Lock()
{
	// TODO: when it's NOT editable, call CreateBuffer.
	void* ptr;
	buffer_->Map(0, nullptr, &ptr);
	return ptr;
}

void TextureDX12::Unlock()
{
	buffer_->Unmap(0, nullptr);

	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	D3D12_TEXTURE_COPY_LOCATION src = {}, dst = {};
	D3D12_RESOURCE_BARRIER barrier;

	auto commandQueue = graphics_->GetCommandQueue();

	auto hr = graphics_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = graphics_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		SafeRelease(commandAllocator);
		goto FAILED_EXIT;
	}

	src.pResource = buffer_;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = footprint;

	dst.pResource = texture_;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	memset(&barrier, 0, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture_;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();
	ID3D12CommandList* list[] = {commandList};
	graphics_->GetCommandQueue()->ExecuteCommandLists(1, list);
	return;

FAILED_EXIT:
	SafeRelease(commandList);
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