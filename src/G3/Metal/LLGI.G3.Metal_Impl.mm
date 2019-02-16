
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "LLGI.G3.Metal_Impl.h"

namespace LLGI
{
namespace G3
{

void PipelineState_Impl::Compile(Graphics_Impl* graphics)
{
	MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

	NSError* pipelineError = nil;
	pipelineState = [graphics->device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&pipelineError];
}

Buffer_Impl::Buffer_Impl() {}

Buffer_Impl::~Buffer_Impl()
{
	if (buffer != nullptr)
	{
		[buffer release];
		buffer = nullptr;
	}
}

bool Buffer_Impl::Initialize(Graphics_Impl* graphics, int32_t size)
{
	buffer = [graphics->device newBufferWithBytes:nil length:size options:MTLResourceOptionCPUCacheModeDefault];

	size_ = size;

	return true;
}

void* Buffer_Impl::GetBuffer() { return buffer.contents; }

bool Texture_Impl::Initialize(Graphics_Impl* graphics, const Vec2I& size)
{
	MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
																								 width:size.X
																								height:size.Y
																							 mipmapped:YES];

	texture = [graphics->device newTextureWithDescriptor:textureDescriptor];

	return true;
}
}
}
