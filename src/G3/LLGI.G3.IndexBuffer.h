
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class IndexBuffer
{
private:
public:
	IndexBuffer() = default;
	virtual ~IndexBuffer() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetStride();
	virtual int32_t GetCount();
};

}
}