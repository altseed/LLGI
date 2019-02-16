#include "LLGI.G3.VertexBufferMetal.h"
#include "LLGI.G3.Metal_Impl.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
namespace G3
{

VertexBufferMetal::VertexBufferMetal() { impl = new Buffer_Impl(); }

VertexBufferMetal::~VertexBufferMetal() { SafeDelete(impl); }

bool VertexBufferMetal::Initialize(Graphics* graphics, int32_t size)
{
	auto graphics_ = (Graphics_Impl*)graphics;
	return impl->Initialize(graphics_, size);
}

void* VertexBufferMetal::Lock() { return impl->GetBuffer(); }

void* VertexBufferMetal::Lock(int32_t offset, int32_t size)
{
	NSCAssert(0 <= offset && offset + size <= impl->size_, @"Run off the buffer");

	auto buffer_ = static_cast<uint8_t*>(impl->GetBuffer());
	buffer_ += offset;
	return buffer_;
}

void VertexBufferMetal::Unlock() {}

int32_t VertexBufferMetal::GetSize() { return impl->size_; }

}
}
