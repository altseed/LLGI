
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class VertexBuffer : public ReferenceObject
{
private:
public:
	VertexBuffer() = default;
	virtual ~VertexBuffer() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetSize();
};

} // namespace LLGI