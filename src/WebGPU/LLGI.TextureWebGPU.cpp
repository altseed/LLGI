#include "LLGI.TextureWebGPU.h"

#include <dawn/platform/DawnPlatform.h>

namespace LLGI
{

bool TextureWebGPU::Initialize(wgpu::Device& device, const TextureParameter& parameter)
{
	device_ = device;

	const auto getDimension = [](int dimension)
	{
		if (dimension == 1)
			return wgpu::TextureDimension::e1D;

		if (dimension == 2)
			return wgpu::TextureDimension::e2D;

		if (dimension == 3)
			return wgpu::TextureDimension::e3D;

		throw "Not implemented";
	};

	const auto getViewDimension = [](int dimension)
	{
		if (dimension == 1)
			return wgpu::TextureViewDimension::e1D;

		if (dimension == 2)
			return wgpu::TextureViewDimension::e2D;

		if (dimension == 3)
			return wgpu::TextureViewDimension::e3D;

		throw "Not implemented";
	};

	{
		wgpu::TextureDescriptor texDesc{};

		texDesc.usage = wgpu::TextureUsage::None;
		if ((parameter.Usage & TextureUsageType::RenderTarget) != TextureUsageType::NoneFlag)
		{
			texDesc.usage |= wgpu::TextureUsage::RenderAttachment;
		}

		if(BitwiseContains(parameter.Usage, TextureUsageType::Storage))
		{
			texDesc.usage |= wgpu::TextureUsage::StorageBinding;
		}

		if ((parameter.Usage & TextureUsageType::External) != TextureUsageType::NoneFlag)
		{
			throw "Not implemented";
			// texDesc.usage |= dawn::platform::kPresentTextureUsage;
		}

		bool isArray = false;
		if ((parameter.Usage & TextureUsageType::Array) != TextureUsageType::NoneFlag)
		{
			isArray = true;
		}

		texDesc.dimension = getDimension(parameter.Dimension);
		texDesc.format = ConvertFormat(parameter.Format);
		texDesc.mipLevelCount = parameter.MipLevelCount;
		texDesc.sampleCount = parameter.SampleCount;
		texDesc.size.width = parameter.Size.X;
		texDesc.size.height = parameter.Size.Y;
		texDesc.size.depthOrArrayLayers = parameter.Size.Z;

		texture_ = device.CreateTexture(&texDesc);
		if (texture_ == nullptr)
		{
			return false;
		}

		wgpu::TextureViewDescriptor texViewDesc{};
		texViewDesc.format = texDesc.format;
		texViewDesc.dimension = getViewDimension(parameter.Dimension);
		texViewDesc.baseMipLevel = 0;
		texViewDesc.mipLevelCount = texDesc.mipLevelCount;
		texViewDesc.baseArrayLayer = 0;
		texViewDesc.arrayLayerCount = isArray ? parameter.Size.Z : 1;
		texViewDesc.aspect = wgpu::TextureAspect::All;

		textureView_ = texture_.CreateView(&texViewDesc);
	}

	format_ = parameter.Format;
	usage_ = parameter.Usage;
	samplingCount_ = parameter.SampleCount;
	mipmapCount_ = parameter.MipLevelCount;

	return texture_ != nullptr && textureView_ != nullptr;
}

void* TextureWebGPU::Lock()
{
	auto cpuMemorySize = GetTextureMemorySize(format_, parameter_.Size);
	temp_buffer_.resize(cpuMemorySize);
	return temp_buffer_.data();
}

void TextureWebGPU::Unlock()
{
	wgpu::ImageCopyTexture imageCopyTexture{};
	imageCopyTexture.texture = texture_;

	wgpu::TextureDataLayout textureDataLayout;
	textureDataLayout.bytesPerRow =
		parameter_.Size.X * GetTextureMemorySize(format_, parameter_.Size) / (parameter_.Size.Y * parameter_.Size.Z);
	wgpu::Extent3D extent;
	extent.width = parameter_.Size.X;
	extent.height = parameter_.Size.Y;
	extent.depthOrArrayLayers = parameter_.Size.Z;
	device_.GetQueue().WriteTexture(&imageCopyTexture, temp_buffer_.data(), temp_buffer_.size(), &textureDataLayout, &extent);
}

} // namespace LLGI