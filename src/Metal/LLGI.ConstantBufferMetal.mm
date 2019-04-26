#include "LLGI.ConstantBufferMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.Metal_Impl.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

ConstantBufferMetal::ConstantBufferMetal() { impl = new Buffer_Impl(); }

ConstantBufferMetal::~ConstantBufferMetal() { SafeDelete(impl); }

bool ConstantBufferMetal::Initialize(Graphics* graphics, int32_t size)
{
	auto graphics_ = static_cast<GraphicsMetal*>(graphics);
	return impl->Initialize(graphics_->GetImpl(), size);
}

void* ConstantBufferMetal::Lock() { return impl->GetBuffer(); }

void* ConstantBufferMetal::Lock(int32_t offset, int32_t size)
{
	NSCAssert(0 <= offset && offset + size <= impl->size_, @"Run off the buffer");

	auto buffer_ = static_cast<uint8_t*>(impl->GetBuffer());
	buffer_ += offset;
	return buffer_;
}

void ConstantBufferMetal::Unlock() {}

int32_t ConstantBufferMetal::GetSize() { return impl->size_; }

}
