
#include "LLGI.G3.ConstantBuffer.h"

namespace LLGI
{
namespace G3
{

void* ConstantBuffer::Lock()
{
	return nullptr;
}

void* ConstantBuffer::Lock(int32_t offset, int32_t size)
{
	return nullptr;
}

void ConstantBuffer::Unlock()
{
}

int32_t ConstantBuffer::GetSize()
{
	return 0;
}

}
}