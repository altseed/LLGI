#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BufferMetal.h"
#import <MetalKit/MetalKit.h>
#include <functional>
#include <unordered_map>

namespace LLGI
{

struct SingleFrameMemoryPoolMetal_Impl;
struct Buffer_Impl;
    
class GraphicsMetal;
class RenderPassMetal;
class RenderPassPipelineStateMetal;
class TextureMetal;

class InternalSingleFrameMemoryPoolMetal
{
private:
    BufferMetal* buffer_ = nullptr;
    int32_t constantBufferSize_ = 0;
    int32_t constantBufferOffset_ = 0;
        
public:
    InternalSingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount);
    virtual ~InternalSingleFrameMemoryPoolMetal();
    bool GetConstantBuffer(int32_t size, Buffer_Impl*& buffer, int32_t& offset);
    void Reset();
};
    
class SingleFrameMemoryPoolMetal : public SingleFrameMemoryPool
{
public:
    SingleFrameMemoryPoolMetal(GraphicsMetal* graphics, int32_t constantBufferPoolSize, int32_t drawingCount);
	virtual ~SingleFrameMemoryPoolMetal();
    virtual void NewFrame() override;
    virtual ConstantBuffer* CreateConstantBuffer(int32_t size) override;
};

} // namespace LLGI
