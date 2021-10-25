#pragma once

#include "../LLGI.Graphics.h"
#include "../LLGI.Buffer.h"
#import <MetalKit/MetalKit.h>
#include <functional>
#include <memory>
#include <unordered_map>

namespace LLGI
{

class BufferMetal : public Buffer
{
private:
	id<MTLBuffer> buffer_ = nullptr;
    id<MTLBuffer> stagingBuffer_ = nullptr;
    id<MTLBuffer> readbackBuffer_ = nullptr;
    int32_t size_ = 0;
    int32_t offset_ = 0;
    bool isExternalResource_ = false;

public:
	BufferMetal();
	~BufferMetal() override;
    
    bool Initialize(Graphics* graphics, BufferUsageType usage, int32_t size);
    bool InitializeAsShortTime(BufferMetal* buffer, int32_t offset, int32_t size);
    
    void* Lock() override;

    void* Lock(int32_t offset, int32_t size) override;

    void Unlock() override;

    void* const Read() override;

    int32_t GetSize() override;
    
    int32_t GetStagingBufferSize() const { return static_cast<int32_t>(stagingBuffer_.length); }
    int32_t GetBufferSize() const { return static_cast<int32_t>(buffer_.length); }
    int32_t GetReadbackBufferSize() const { return static_cast<int32_t>(readbackBuffer_.length); }
    
    int32_t GetOffset() const { return offset_; }

	id<MTLBuffer>& GetBuffer() { return buffer_; }
    id<MTLBuffer>& GetStagingBuffer() { return stagingBuffer_; }
    id<MTLBuffer>& GetReadbackBuffer() {
        return readbackBuffer_;
    }
};

} // namespace LLGI
