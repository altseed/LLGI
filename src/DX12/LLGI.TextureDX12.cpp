#include "LLGI.TextureDX12.h"
#include "LLGI.CommandListDX12.h"

namespace LLGI
{
TextureDX12::TextureDX12(GraphicsDX12* graphics) : graphics_(graphics)
{
	SafeAddRef(graphics_);
	memset(&footprint_, 0, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT));
}

TextureDX12::~TextureDX12()
{
	SafeRelease(texture_);
	SafeRelease(buffer_);
	SafeRelease(graphics_);
}

bool TextureDX12::Initialize(const Vec2I& size, const bool isRenderPass, const bool isDepthBuffer, const TextureFormatType formatType)
{
	isRenderPass_ = isRenderPass;

	if (isDepthBuffer)
		throw "Not implemented";

	DXGI_FORMAT format;
	switch (formatType)
	{
	case TextureFormatType::R8G8B8A8_UNORM:
		format = DXGI_FORMAT_B8G8R8A8_UNORM;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R16G16B16A16_FLOAT:
		format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		memorySize_ = size.X * size.Y * 8;
		break;
	case TextureFormatType::R32G32B32A32_FLOAT:
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		memorySize_ = size.X * size.Y * 16;
		
		break;
	case TextureFormatType::R8G8B8A8_UNORM_SRGB:
		format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R16G16_FLOAT:
		format = DXGI_FORMAT_R16G16_FLOAT;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R8_UNORM:
		format = DXGI_FORMAT_R8_UNORM;
		memorySize_ = size.X * size.Y * 1;
		break;
	default: 
		throw "Not implemented";
	//case TextureFormatType::BC1:
	//	format = DXGI_FORMAT_BC1_UNORM;
	//	break;
	//case TextureFormatType::BC2:
	//	format = DXGI_FORMAT_BC2_UNORM;
	//	break;
	//case TextureFormatType::BC3:
	//	format = DXGI_FORMAT_BC3_UNORM;
	//	break;
	//case TextureFormatType::BC1_SRGB:
	//	format = DXGI_FORMAT_BC1_UNORM_SRGB;
	//	break;
	//case TextureFormatType::BC2_SRGB:
	//	format = DXGI_FORMAT_BC1_UNORM_SRGB;
	//	break;
	//case TextureFormatType::BC3_SRGB:
	//	format = DXGI_FORMAT_BC1_UNORM_SRGB;
	//	break;
	}

	if (isRenderPass_)
	{
		texture_ = graphics_->CreateResource(D3D12_HEAP_TYPE_DEFAULT,
											 format,
											 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
											 D3D12_RESOURCE_STATE_GENERIC_READ,
											 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
											 size);
		state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else
	{
		texture_ = graphics_->CreateResource(
			D3D12_HEAP_TYPE_DEFAULT, format, D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_RESOURCE_STATE_COPY_DEST, size);

		state_ = D3D12_RESOURCE_STATE_COPY_DEST;
	}
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

	ResourceBarrior(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandList->Close();
	ID3D12CommandList* list[] = {commandList};
	graphics_->GetCommandQueue()->ExecuteCommandLists(1, list);

	// TODO optimize it
	graphics_->WaitFinish();
	SafeRelease(commandList);
	SafeRelease(commandAllocator);
	return;

FAILED_EXIT:
	SafeRelease(commandList);
}

Vec2I TextureDX12::GetSizeAs2D() { return textureSize_; }

bool TextureDX12::IsRenderTexture() const { return isRenderPass_; }

bool TextureDX12::IsDepthTexture() const
{
	throw "Not implemented";
	return isDepthBuffer_;
}

void TextureDX12::ResourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES state)
{
	if (state_ == state)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture_;
	barrier.Transition.StateBefore = state_;
	barrier.Transition.StateAfter = state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	state_ = state;
}

} // namespace LLGI