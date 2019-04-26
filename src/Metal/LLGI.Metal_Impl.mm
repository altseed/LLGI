
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#import "LLGI.Metal_Impl.h"

namespace LLGI
{

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
	buffer = [graphics->device newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];

	size_ = size;

	return true;
}

void* Buffer_Impl::GetBuffer() { return buffer.contents; }

}
