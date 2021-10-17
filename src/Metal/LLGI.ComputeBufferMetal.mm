#include "LLGI.ComputeBufferMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.Metal_Impl.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

ComputeBufferMetal::ComputeBufferMetal() {}

ComputeBufferMetal::~ComputeBufferMetal() { SafeRelease(buffer_); }

bool ComputeBufferMetal::Initialize(Graphics* graphics, int32_t size)
{
    auto buffer = new BufferMetal(graphics, size);
    SafeAssign(buffer_, buffer);
    SafeRelease(buffer);
    size_ = size;
    offset_ = 0;
    return true;
}

void* ComputeBufferMetal::Lock()
{
    auto buffer = static_cast<uint8_t*>(buffer_->GetData());
    buffer += offset_;
    return buffer;
}

void* ComputeBufferMetal::Lock(int32_t offset, int32_t size)
{
    NSCAssert(0 <= offset && offset + offset_ + size <= buffer_->GetSize(), @"Run off the buffer");

    auto buffer = static_cast<uint8_t*>(buffer_->GetData());
    buffer += offset + offset_;
    return buffer;
}

void ComputeBufferMetal::Unlock() {}

void* const ComputeBufferMetal::Read() {
    return buffer_->GetData();
}

int32_t ComputeBufferMetal::GetSize() { return buffer_->GetSize(); }

}
