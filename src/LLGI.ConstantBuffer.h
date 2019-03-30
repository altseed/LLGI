
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class ConstantBuffer : public ReferenceObject
{
private:
public:
	ConstantBuffer() = default;
	virtual ~ConstantBuffer() = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();
	virtual int32_t GetSize();
};

} // namespace LLGI