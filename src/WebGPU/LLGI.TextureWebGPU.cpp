#include "LLGI.TextureWebGPU.h"

namespace LLGI
{

bool TextureWebGPU::Initialize(wgpu::Device& device, const TextureParameter& parameter)
{
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

		if ((parameter.Usage & TextureUsageType::External) != TextureUsageType::NoneFlag)
		{
			texDesc.usage |= wgpu::TextureUsage::Present;
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
	
	samplingCount_ = parameter.SampleCount;

	mipmapCount_ = parameter.MipLevelCount;

	throw "Not implemented (Copy cpu to gpu)";

	return texture_ != nullptr && textureView_ != nullptr;
}

} // namespace LLGI