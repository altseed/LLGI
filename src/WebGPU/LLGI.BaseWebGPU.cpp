#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

wgpu::TextureFormat ConvertFormat(TextureFormatType format)
{
	if (format == TextureFormatType::R8G8B8A8_UNORM)
		return wgpu::TextureFormat::RGBA8Unorm;

	if (format == TextureFormatType::R16G16B16A16_FLOAT)
		return wgpu::TextureFormat::RGBA16Float;

	if (format == TextureFormatType::R32G32B32A32_FLOAT)
		return wgpu::TextureFormat::RGBA32Float;

	if (format == TextureFormatType::R8G8B8A8_UNORM_SRGB)
		return wgpu::TextureFormat::RGBA8UnormSrgb;

	if (format == TextureFormatType::R16G16_FLOAT)
		return wgpu::TextureFormat::RG16Float;

	if (format == TextureFormatType::R8_UNORM)
		return wgpu::TextureFormat::R8Unorm;

	if (format == TextureFormatType::BC1)
		return wgpu::TextureFormat::BC1RGBAUnorm;

	if (format == TextureFormatType::BC2)
		return wgpu::TextureFormat::BC2RGBAUnorm;

	if (format == TextureFormatType::BC3)
		return wgpu::TextureFormat::BC3RGBAUnorm;

	if (format == TextureFormatType::BC1_SRGB)
		return wgpu::TextureFormat::BC1RGBAUnormSrgb;

	if (format == TextureFormatType::BC2_SRGB)
		return wgpu::TextureFormat::BC2RGBAUnormSrgb;

	if (format == TextureFormatType::BC3_SRGB)
		return wgpu::TextureFormat::BC3RGBAUnormSrgb;

	if (format == TextureFormatType::D32)
		return wgpu::TextureFormat::Depth32Float;

	if (format == TextureFormatType::D24S8)
		return wgpu::TextureFormat::Depth24UnormStencil8;

	if (format == TextureFormatType::D32S8)
		return wgpu::TextureFormat::Depth32FloatStencil8;

	if (format == TextureFormatType::Unknown)
		return wgpu::TextureFormat::Undefined;

	throw "Not implemented";
}

TextureFormatType ConvertFormat(wgpu::TextureFormat format)
{
	if (format == wgpu::TextureFormat::RGBA8Unorm)
		return TextureFormatType::R8G8B8A8_UNORM;

	if (format == wgpu::TextureFormat::RGBA16Float)
		return TextureFormatType::R16G16B16A16_FLOAT;

	if (format == wgpu::TextureFormat::RGBA32Float)
		return TextureFormatType::R32G32B32A32_FLOAT;

	if (format == wgpu::TextureFormat::RGBA8UnormSrgb)
		return TextureFormatType::R8G8B8A8_UNORM_SRGB;

	if (format == wgpu::TextureFormat::RG16Float)
		return TextureFormatType::R16G16_FLOAT;

	if (format == wgpu::TextureFormat::R8Unorm)
		return TextureFormatType::R8_UNORM;

	if (format == wgpu::TextureFormat::BC1RGBAUnorm)
		return TextureFormatType::BC1;

	if (format == wgpu::TextureFormat::BC2RGBAUnorm)
		return TextureFormatType::BC2;

	if (format == wgpu::TextureFormat::BC3RGBAUnorm)
		return TextureFormatType::BC3;

	if (format == wgpu::TextureFormat::BC1RGBAUnormSrgb)
		return TextureFormatType::BC1_SRGB;

	if (format == wgpu::TextureFormat::BC2RGBAUnormSrgb)
		return TextureFormatType::BC2_SRGB;

	if (format == wgpu::TextureFormat::BC3RGBAUnormSrgb)
		return TextureFormatType::BC3_SRGB;

	if (format == wgpu::TextureFormat::Depth32Float)
		return TextureFormatType::D32;

	if (format == wgpu::TextureFormat::Depth24UnormStencil8)
		return TextureFormatType::D24S8;

	if (format == wgpu::TextureFormat::Depth32FloatStencil8)
		return TextureFormatType::D32S8;

	if (format == wgpu::TextureFormat::Undefined)
		return TextureFormatType::Unknown;

	throw "Not implemented";
}

} // namespace LLGI