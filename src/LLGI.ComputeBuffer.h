#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class ComputeBuffer : public ReferenceObject
{
private:
public:
	ComputeBuffer() = default;
	~ComputeBuffer() override = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();

	virtual int32_t GetSize();
};

} // namespace LLGI
