
#pragma once

#include "../LLGI.Texture.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

namespace LLGI
{

class TextureDX12 : public Texture
{
private:
	GraphicsDX12* graphics_ = nullptr;
	ID3D12Resource* texture_ = nullptr;

	ID3D12Resource* buffer_ = nullptr;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint_;
	D3D12_RESOURCE_STATES state_ = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	DXGI_FORMAT format_;
	TextureFormatType formatType_;

	Vec2I textureSize_;
	int32_t memorySize_;

	bool isRenderPass_ = false;
	bool isDepthBuffer_ = false;

	void CreateBuffer();

public:
	TextureDX12(GraphicsDX12* graphics);
	virtual ~TextureDX12();

	bool Initialize(const Vec2I& size, const bool isRenderPass, const bool isDepthBuffer, const TextureFormatType formatType);

	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	ID3D12Resource* Get() const { return texture_; }
	int32_t GetMemorySize() const { return memorySize_; }
	TextureFormatType GetFormat() const override { return formatType_; }
	DXGI_FORMAT GetDXGIFormat() const { return format_; }
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;

	void ResourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES state);
};
} // namespace LLGI