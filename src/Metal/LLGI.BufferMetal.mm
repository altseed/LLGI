#include "LLGI.BufferMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.PipelineStateMetal.h"
#include "LLGI.RenderPassMetal.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#include "LLGI.TextureMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

BufferMetal::BufferMetal()
{
	
}

BufferMetal::~BufferMetal()
{
    if (isExternalResource_)
        return;
    
	if (buffer_ != nullptr)
	{
		[buffer_ release];
		buffer_ = nullptr;
	}
    
    if (stagingBuffer_ != nullptr)
    {
        [stagingBuffer_ release];
        stagingBuffer_ = nullptr;
    }
    
    if (readbackBuffer_ != nullptr)
    {
        [readbackBuffer_ release];
        readbackBuffer_ = nullptr;
    }
    
}

bool BufferMetal::Initialize(Graphics* graphics, BufferUsageType usage, int32_t size)
{
    auto g = static_cast<GraphicsMetal*>(graphics);
    buffer_ = [g->GetDevice() newBufferWithLength:size options:MTLResourceStorageModePrivate];
    
    stagingBuffer_ = [g->GetDevice() newBufferWithLength:size options:MTLResourceStorageModeShared];
    
    readbackBuffer_ = [g->GetDevice() newBufferWithLength:size options:MTLResourceStorageModeShared];
    
    size_ = size;
    
    return true;
}

bool BufferMetal::InitializeAsShortTime(BufferMetal* buffer, int32_t offset, int32_t size)
{
    buffer_ = buffer->GetBuffer();
    stagingBuffer_ = buffer->GetStagingBuffer();
    readbackBuffer_ = buffer->GetReadbackBuffer();
    
    size_ = size;
    offset_ = offset;
    isExternalResource_ = true;
    
    return true;
}

void* BufferMetal::Lock()
{
    auto buffer = static_cast<uint8_t*>(stagingBuffer_.contents);
    buffer += offset_;
    return buffer;
}

void* BufferMetal::Lock(int32_t offset, int32_t size)
{
    NSCAssert(0 <= offset && offset + offset_ + size <= GetStagingBufferSize(), @"Run off the buffer");

    auto buffer = static_cast<uint8_t*>(stagingBuffer_.contents);
    buffer += offset + offset_;
    return buffer;
}

void BufferMetal::Unlock() {}

void* const BufferMetal::Read() { return readbackBuffer_.contents; }

int32_t BufferMetal::GetSize() { return size_; }

}
