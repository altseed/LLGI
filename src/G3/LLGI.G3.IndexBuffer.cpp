
#include "LLGI.G3.IndexBuffer.h"

namespace LLGI
{
namespace G3
{

void* IndexBuffer::Lock()
{
	return nullptr;
}

void* IndexBuffer::Lock(int32_t offset, int32_t size)
{
	return nullptr;
}

void IndexBuffer::Unlock()
{
}

int32_t IndexBuffer::GetStride()
{
	return 0;
}

int32_t IndexBuffer::GetCount()
{
	return 0;
}

}
}