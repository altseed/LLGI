
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

	ID3D12DescriptorHeap* SrvDescriptorHeap_ = nullptr;
	ID3D12DescriptorHeap* samplerDescriptorHeap_ = nullptr;

	Vec2I textureSize_;

	bool isRenderPass_ = false;
	bool isDepthBuffer_ = false;

	void CreateBuffer();

public:
	TextureDX12(GraphicsDX12* graphics);
	virtual ~TextureDX12();

	bool Initialize(const Vec2I& size, bool isRenderPass, bool isDepthBuffer);

	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	ID3D12Resource* Get() { return texture_; }
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;

	void CreateView();
	void CreateSampler(TextureWrapMode wrapMode);
	ID3D12DescriptorHeap* GetSrv() { return SrvDescriptorHeap_; }
	ID3D12DescriptorHeap* GetSampler() { return samplerDescriptorHeap_; }
};
} // namespace LLGI