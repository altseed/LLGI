#include "LLGI.TextureMetal.h"
#include "LLGI.Metal_Impl.h"
#include <TargetConditionals.h>

namespace LLGI
{

bool TextureMetal::Initialize(
	id<MTLDevice> device, const Vec3I& size, TextureFormatType format, TextureUsageType usage, int dimension, int samplingCount, TextureType type, int MipMapCount)
{
	MTLTextureDescriptor* textureDescriptor = nullptr;

	bool isMipmapped = MipMapCount >= 2;

	if (type == TextureType::Depth)
	{
		textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:ConvertFormat(format)
																			   width:size.X
																			  height:size.Y
																		   mipmapped:isMipmapped];
		textureDescriptor.usage = MTLTextureUsageRenderTarget;
		textureDescriptor.textureType = MTLTextureType2D;
		textureDescriptor.storageMode = MTLStorageModePrivate;
		textureDescriptor.sampleCount = samplingCount;

		if (samplingCount > 1)
		{
			textureDescriptor.textureType = MTLTextureType2DMultisample;
			textureDescriptor.storageMode = MTLStorageModePrivate;
		}
	}
	else
	{
		textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:ConvertFormat(format)
																			   width:size.X
																			  height:size.Y
																		   mipmapped:isMipmapped];
        
        textureDescriptor = [[[MTLTextureDescriptor alloc] init] autorelease];
        
        const bool isArray = (usage & TextureUsageType::Array) != TextureUsageType::None;
        
        if(dimension == 3)
        {
            textureDescriptor.textureType = MTLTextureType3D;
        }
        else if(dimension == 2)
        {
            if(isArray)
            {
                if(samplingCount > 1)
                {
                    textureDescriptor.textureType = MTLTextureType2DMultisampleArray;
                }
                else
                {
                    textureDescriptor.textureType = MTLTextureType2DArray;
                }
            }
            else
            {
                if(samplingCount > 1)
                {
                    textureDescriptor.textureType = MTLTextureType2DMultisample;
                }
                else
                {
                    textureDescriptor.textureType = MTLTextureType2D;
                }
            }
        }
        else
        {
            throw "1d texture is not supported";
        }
        
        textureDescriptor.width = size.X;
        textureDescriptor.height = size.Y;
        textureDescriptor.depth = isArray ? 1 : size.Z;
        textureDescriptor.arrayLength = isArray ? size.Z : 1;
        textureDescriptor.pixelFormat = ConvertFormat(format);
        textureDescriptor.sampleCount = samplingCount;
        textureDescriptor.mipmapLevelCount = isMipmapped ? GetMaximumMipLevels(Vec2I{size.X,size.Y}) : 1;
        
        if (samplingCount > 1)
        {
            textureDescriptor.textureType = MTLTextureType2DMultisample;
            textureDescriptor.storageMode = MTLStorageModePrivate;
        }
    }

	if (isMipmapped)
	{
		textureDescriptor.mipmapLevelCount = MipMapCount;
	}

	texture_ = [device newTextureWithDescriptor:textureDescriptor];
    
	size_ = size;

	fromExternal_ = false;

	return true;
}

void TextureMetal::Write(const uint8_t* data)
{
	MTLRegion region = {{0, 0, 0}, {static_cast<uint32_t>(size_.X), static_cast<uint32_t>(size_.Y), 1}};

    auto allSize = GetTextureMemorySize(ConvertFormat(texture_.pixelFormat), Vec3I{size_.X, size_.Y, 1});

	[texture_ replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:allSize / size_.Y];
}

TextureMetal::TextureMetal() {}

TextureMetal::~TextureMetal()
{
	if (texture_ != nullptr)
	{
		[texture_ release];
		texture_ = nullptr;
	}

	SafeRelease(owner_);
}

bool TextureMetal::Initialize(GraphicsMetal* owner, const TextureParameter& parameter)
{
    type_ = TextureType::Color;
    if (!Initialize(owner->GetDevice(), parameter.Size, parameter.Format, parameter.Usage, parameter.Dimension, parameter.SampleCount, type_, parameter.MipLevelCount))
    {
        return false;
    }

    format_ = ConvertFormat(texture_.pixelFormat);
    data.resize(GetTextureMemorySize(format_, parameter.Size));
    parameter_ = parameter;    
    return true;
}

bool TextureMetal::Initialize(GraphicsMetal* owner, const RenderTextureInitializationParameter& parameter)
{
	type_ = TextureType::Render;
	samplingCount_ = parameter.SamplingCount;

	SafeAssign(owner_, static_cast<ReferenceObject*>(owner));

	id<MTLDevice> device = owner->GetDevice();
	MTLTextureDescriptor* textureDescriptor = nullptr;

	textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:ConvertFormat(parameter.Format)
																		   width:parameter.Size.X
																		  height:parameter.Size.Y
																	   mipmapped:NO];
	textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
	textureDescriptor.depth = 1;

	if (parameter.SamplingCount > 1)
	{
		textureDescriptor.textureType = MTLTextureType2DMultisample;
		textureDescriptor.storageMode = MTLStorageModePrivate;
	}
	else
	{
		textureDescriptor.textureType = MTLTextureType2D;
		// Make copy enabled in GetBuffer
		// TODO : Optimize
#if !(TARGET_OS_IPHONE) && !(TARGET_OS_SIMULATOR)
		textureDescriptor.storageMode = MTLStorageModeManaged;
#else
		textureDescriptor.storageMode = MTLStorageModePrivate;
#endif
	}

	textureDescriptor.sampleCount = parameter.SamplingCount;

	texture_ = [device newTextureWithDescriptor:textureDescriptor];

    size_ = Vec3I{parameter.Size.X, parameter.Size.Y, 1};

	fromExternal_ = false;

	format_ = ConvertFormat(texture_.pixelFormat);
    data.resize(GetTextureMemorySize(format_, size_));

	return true;
}

bool TextureMetal::Initialize(GraphicsMetal* owner, const DepthTextureInitializationParameter& parameter)
{
	type_ = TextureType::Depth;

	/*
	 #if TARGET_OS_IOS
			 auto format = MTLPixelFormatDepth32Float_Stencil8;
	 #elif TARGET_OS_MAC
			 bool supported = device.isDepth24Stencil8PixelFormatSupported;

			 auto format = (supported) ? MTLPixelFormatDepth24Unorm_Stencil8 : MTLPixelFormatDepth32Float_Stencil8;
	 #endif
	 **/

	samplingCount_ = parameter.SamplingCount;

	SafeAssign(owner_, static_cast<ReferenceObject*>(owner));

	TextureFormatType format = TextureFormatType::D32;
	if (parameter.Mode == DepthTextureMode::DepthStencil)
	{
		format = TextureFormatType::D24S8;

		if (!owner->GetDevice().isDepth24Stencil8PixelFormatSupported)
		{
			return false;
		}
	}

    const auto mergedSize = Vec3I{size_.X, size_.Y, 1};
	if (!Initialize(owner->GetDevice(), mergedSize, format, TextureUsageType::None, 2, parameter.SamplingCount, type_, 1))
	{
		return false;
	}

	format_ = ConvertFormat(texture_.pixelFormat);
    data.resize(GetTextureMemorySize(format_, mergedSize));

	return true;
}

bool TextureMetal::Initialize(GraphicsMetal* owner, id<MTLTexture> externalTexture)
{
	if (externalTexture == nullptr)
	{
		return false;
	}

	Reset(externalTexture);
	type_ = TextureType::Color;

	format_ = ConvertFormat(texture_.pixelFormat);

	return true;
}

void TextureMetal::Reset(id<MTLTexture> nativeTexture)
{
	type_ = TextureType::Screen;

	if (nativeTexture != nullptr)
	{
		[nativeTexture retain];
	}

	if (texture_ != nullptr)
	{
		[texture_ release];
	}

	texture_ = nativeTexture;

	if (texture_ != nullptr)
	{
		size_.X = static_cast<int32_t>(texture_.width);
		size_.Y = static_cast<int32_t>(texture_.height);
	}
	else
	{
		size_.X = 0.0f;
		size_.Y = 0.0f;
	}

	fromExternal_ = true;

	format_ = ConvertFormat(texture_.pixelFormat);
}

void* TextureMetal::Lock() { return data.data(); }

void TextureMetal::Unlock() { Write(data.data()); }

Vec2I TextureMetal::GetSizeAs2D() const { return Vec2I{size_.X, size_.Y}; }

}
