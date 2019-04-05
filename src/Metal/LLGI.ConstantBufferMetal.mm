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

void* ConstantBufferMetal::Lock() { throw "Not inplemented"; }

void* ConstantBufferMetal::Lock(int32_t offset, int32_t size) { throw "Not inplemented"; }

void ConstantBufferMetal::Unlock() { throw "Not inplemented"; }

int32_t ConstantBufferMetal::GetSize() { throw "Not inplemented"; }

}
