#include "LLGI.TextureDX12.h"
#include "LLGI.CommandListDX12.h"

namespace LLGI
{

TextureDX12::TextureDX12(GraphicsDX12* graphics, bool hasStrongRef) : graphics_(graphics), hasStrongRef_(hasStrongRef)
{
	if (hasStrongRef_)
	{
		SafeAddRef(graphics_);
	}

	assert(graphics_ != nullptr);
	device_ = graphics_->GetDevice();
	commandQueue_ = graphics_->GetCommandQueue();

	SafeAddRef(device_);
	SafeAddRef(commandQueue_);
	memset(&footprint_, 0, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT));
}

TextureDX12::TextureDX12(ID3D12Resource* textureResource, ID3D12Device* device, ID3D12CommandQueue* commandQueue)
	: texture_(textureResource), commandQueue_(commandQueue), device_(device)
{
	SafeAddRef(texture_);
	SafeAddRef(device_);
	SafeAddRef(commandQueue_);
	memset(&footprint_, 0, sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT));

	auto desc = texture_->GetDesc();
	dxgiFormat_ = desc.Format;

	switch (desc.Format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		format_ = TextureFormatType::R8G8B8A8_UNORM;
		memorySize_ = desc.Width * desc.Height * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		format_ = TextureFormatType::R16G16B16A16_FLOAT;
		memorySize_ = desc.Width * desc.Height * 8;
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		format_ = TextureFormatType::R32G32B32A32_FLOAT;
		memorySize_ = desc.Width * desc.Height * 16;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		format_ = TextureFormatType::R8G8B8A8_UNORM_SRGB;
		memorySize_ = desc.Width * desc.Height * 4;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
		format_ = TextureFormatType::R16G16_FLOAT;
		memorySize_ = desc.Width * desc.Height * 4;
		break;
	case DXGI_FORMAT_R8_UNORM:
		format_ = TextureFormatType::R8_UNORM;
		memorySize_ = desc.Width * desc.Height * 1;
		break;
	default:
		throw "Not implemented";
		// case DXGI_FORMAT_BC1:
		//	format_=TextureFormatType::BC1_UNORM;
		//	break;
		// case DXGI_FORMAT_BC2:
		//	format_=TextureFormatType::BC2_UNORM;
		//	break;
		// case DXGI_FORMAT_BC3:
		//	format_=TextureFormatType::BC3_UNORM;
		//	break;
		// case DXGI_FORMAT_BC1_SRGB:
		//	format_=TextureFormatType::BC1_UNORM_SRGB;
		//	break;
		// case DXGI_FORMAT_BC2_SRGB:
		//	format_=TextureFormatType::BC1_UNORM_SRGB;
		//	break;
		// case DXGI_FORMAT_BC3_SRGB:
		//	format_=TextureFormatType::BC1_UNORM_SRGB;
		//	break;
	}

	textureSize_ = Vec2I(desc.Width, desc.Height);
}

TextureDX12::~TextureDX12()
{
	SafeRelease(texture_);
	SafeRelease(buffer_);

	if (hasStrongRef_)
	{
		SafeRelease(graphics_);
	}
	SafeRelease(device_);
	SafeRelease(commandQueue_);
}

bool TextureDX12::Initialize(const Vec2I& size, const bool isRenderPass, const bool isDepthBuffer, const TextureFormatType formatType)
{
	isRenderPass_ = isRenderPass;

	if (isDepthBuffer)
		throw "Not implemented";

	switch (formatType)
	{
	case TextureFormatType::R8G8B8A8_UNORM:
		dxgiFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R16G16B16A16_FLOAT:
		dxgiFormat_ = DXGI_FORMAT_R16G16B16A16_FLOAT;
		memorySize_ = size.X * size.Y * 8;
		break;
	case TextureFormatType::R32G32B32A32_FLOAT:
		dxgiFormat_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
		memorySize_ = size.X * size.Y * 16;
		break;
	case TextureFormatType::R8G8B8A8_UNORM_SRGB:
		dxgiFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R16G16_FLOAT:
		dxgiFormat_ = DXGI_FORMAT_R16G16_FLOAT;
		memorySize_ = size.X * size.Y * 4;
		break;
	case TextureFormatType::R8_UNORM:
		dxgiFormat_ = DXGI_FORMAT_R8_UNORM;
		memorySize_ = size.X * size.Y * 1;
		break;
	default:
		throw "Not implemented";
		// case TextureFormatType::BC1:
		//	dxgiFormat_ = DXGI_FORMAT_BC1_UNORM;
		//	break;
		// case TextureFormatType::BC2:
		//	dxgiFormat_ = DXGI_FORMAT_BC2_UNORM;
		//	break;
		// case TextureFormatType::BC3:
		//	dxgiFormat_ = DXGI_FORMAT_BC3_UNORM;
		//	break;
		// case TextureFormatType::BC1_SRGB:
		//	dxgiFormat_ = DXGI_FORMAT_BC1_UNORM_SRGB;
		//	break;
		// case TextureFormatType::BC2_SRGB:
		//	dxgiFormat_ = DXGI_FORMAT_BC1_UNORM_SRGB;
		//	break;
		// case TextureFormatType::BC3_SRGB:
		//	dxgiFormat_ = DXGI_FORMAT_BC1_UNORM_SRGB;
		//	break;
	}
	format_ = formatType;

	if (isRenderPass_)
	{
		texture_ = CreateResourceBuffer(device_,
										D3D12_HEAP_TYPE_DEFAULT,
										dxgiFormat_,
										D3D12_RESOURCE_DIMENSION_TEXTURE2D,
										D3D12_RESOURCE_STATE_GENERIC_READ,
										D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
										size);
		state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else
	{
		texture_ = CreateResourceBuffer(device_,
										D3D12_HEAP_TYPE_DEFAULT,
										dxgiFormat_,
										D3D12_RESOURCE_DIMENSION_TEXTURE2D,
										D3D12_RESOURCE_STATE_COPY_DEST,
										D3D12_RESOURCE_FLAG_NONE,
										size);

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
	device_->GetCopyableFootprints(&texture_->GetDesc(), 0, 1, 0, &footprint_, nullptr, nullptr, &size);

	buffer_ = CreateResourceBuffer(device_,
								   D3D12_HEAP_TYPE_UPLOAD,
								   DXGI_FORMAT_UNKNOWN,
								   D3D12_RESOURCE_DIMENSION_BUFFER,
								   D3D12_RESOURCE_STATE_GENERIC_READ,
								   D3D12_RESOURCE_FLAG_NONE,
								   Vec2I(size, 1));
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

	auto hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
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
	commandQueue_->ExecuteCommandLists(1, list);

	// TODO optimize it
	if (graphics_ != nullptr)
	{
		graphics_->WaitFinish();
	}

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