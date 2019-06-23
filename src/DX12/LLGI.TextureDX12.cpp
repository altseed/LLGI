#include "LLGI.TextureDX12.h"
#include "LLGI.CommandListDX12.h"

namespace LLGI
{
TextureDX12::TextureDX12(GraphicsDX12* graphics) : graphics_(graphics) { SafeAddRef(graphics_); }

TextureDX12::~TextureDX12()
{
	SafeRelease(texture_);
	SafeRelease(buffer_);
	SafeRelease(graphics_);
}

bool TextureDX12::Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer)
{
	isRenderPass_ = isRenderPass;

	if (isDepthBuffer)
		throw "Not implemented";

	if (isRenderPass_)
		texture_ = graphics_->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											 DXGI_FORMAT_R8G8B8A8_UNORM,
											 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
											 D3D12_RESOURCE_STATE_COPY_DEST,
											 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
											 size);
	else
		texture_ = graphics_->CreateResource(
			D3D12_HEAP_TYPE_DEFAULT, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_COPY_DEST, size);

	textureSize_ = size;
	if (texture_ == nullptr)
		return false;

	// TODO: when it's NOT editable, do NOT call CreateBuffer.
	CreateBuffer();

	return true;
}

void TextureDX12::CreateBuffer()
{
	UINT64 size = 0;
	graphics_->GetDevice()->GetCopyableFootprints(&texture_->GetDesc(), 0, 1, 0, &footprint_, nullptr, nullptr, &size);

	buffer_ = graphics_->CreateResource(
		D3D12_HEAP_TYPE_UPLOAD, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_STATE_GENERIC_READ, Vec2I(size, 1));
	assert(buffer_ != nullptr);
}

void* TextureDX12::Lock()
{
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
	src.PlacedFootprint = footprint_;

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

Vec2I TextureDX12::GetSizeAs2D() { return textureSize_; }

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