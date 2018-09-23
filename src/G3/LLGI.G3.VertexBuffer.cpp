
#include "LLGI.G3.VertexBuffer.h"

namespace LLGI
{
namespace G3
{

void* VertexBuffer::Lock()
{
	return nullptr;
}

void* VertexBuffer::Lock(int32_t offset, int32_t size)
{
	return nullptr;
}

void VertexBuffer::Unlock()
{
}

int32_t VertexBuffer::GetSize()
{
	return 0;
}

}
}