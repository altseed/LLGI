#pragma once

#include "../LLGI.Graphics.h"
#import <MetalKit/MetalKit.h>
#include <functional>
#include <unordered_map>
#include <memory>

namespace LLGI
{

class BufferMetal : public ReferenceObject
{
private:
    id<MTLBuffer> buffer_ = nullptr;
    int32_t size_;
    
public:
	BufferMetal(Graphics* graphics, int32_t size);
    ~BufferMetal() override;
    
	void* GetData()
    {
        return buffer_.contents;
    }

    id<MTLBuffer>& GetBuffer()
    {
        return buffer_;
    }
    
    int32_t GetSize() const { return size_; }
};

} // namespace LLGI
