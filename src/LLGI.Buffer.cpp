#include "LLGI.Buffer.h"

namespace LLGI
{

void* Buffer::Lock() { return nullptr; }

void* Buffer::Lock(int32_t offset, int32_t size) { return nullptr; }

void Buffer::Unlock() {}

void* const Buffer::Read() { return nullptr; }

int32_t Buffer::GetSize() { return int32_t(); }

} // namespace LLGI
