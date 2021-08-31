#include "LLGI.ComputeBuffer.h"

namespace LLGI
{
void* ComputeBuffer::Lock() { return nullptr; }

void* ComputeBuffer::Lock(int32_t offset, int32_t size) { return nullptr; }

void ComputeBuffer::Unlock() {}

int32_t ComputeBuffer::GetSize() { return int32_t(); }
} // namespace LLGI
