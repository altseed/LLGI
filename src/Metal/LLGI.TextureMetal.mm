#include "LLGI.TextureMetal.h"
#include "LLGI.Metal_Impl.h"

namespace LLGI
{

Texture_Impl::Texture_Impl() {}

Texture_Impl::~Texture_Impl()
{
	if (texture != nullptr)
	{
		[texture release];
		texture = nullptr;
	}
}

bool Texture_Impl::Initialize(Graphics_Impl* graphics, const Vec2I& size, bool isRenderTexture, bool isDepthTexture)
{
    MTLTextureDescriptor* textureDescriptor = nullptr;
    
    if(isDepthTexture)
    {
        textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth24Unorm_Stencil8
                                                                                                     width:size.X
                                                                                                    height:size.Y
                                                                                                 mipmapped:YES];

    }
    else
    {
        textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
																								 width:size.X
																								height:size.Y
																							 mipmapped:YES];
    }
    
    if(isRenderTexture)
    {
        textureDescriptor.usage = MTLTextureUsageRenderTarget;
    }
    
	texture = [graphics->device newTextureWithDescriptor:textureDescriptor];

	size_ = size;

	return true;
}

void Texture_Impl::Write(const uint8_t* data)
{
	MTLRegion region = {{0, 0, 0}, {static_cast<uint32_t>(size_.X), static_cast<uint32_t>(size_.Y), 1}};

	[texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:size_.X * 4];
}

TextureMetal::TextureMetal() { impl = new Texture_Impl(); }

TextureMetal::~TextureMetal()
{
	SafeDelete(impl);
	SafeRelease(graphics_);
}

bool TextureMetal::Initialize(Graphics* graphics, Vec2I size, bool isRenderTexture, bool isDepthTexture)
{
    isRenderTexture_ = isRenderTexture;
    isDepthTexture_ = isDepthTexture;
    
	graphics_ = static_cast<GraphicsMetal*>(graphics);
	SafeAddRef(graphics_);

	data.resize(size.X * size.Y * 4);
	return impl->Initialize(graphics_->GetImpl(), size, isRenderTexture_, isDepthTexture_);
}

void* TextureMetal::Lock() { return data.data(); }

void TextureMetal::Unlock() { impl->Write(data.data()); }

Vec2I TextureMetal::GetSizeAs2D() { return impl->size_; }

bool TextureMetal::IsRenderTexture() const { return isRenderTexture_; }

bool TextureMetal::IsDepthTexture() const { return isDepthTexture_; }

Texture_Impl* TextureMetal::GetImpl() const { return impl; }

}
