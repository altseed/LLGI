#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class Buffer : public ReferenceObject
{
private:
public:
	Buffer() = default;
	~Buffer() override = default;

	virtual void* Lock();
	virtual void* Lock(int32_t offset, int32_t size);
	virtual void Unlock();

	virtual void* const Read();

	virtual int32_t GetSize();
};

} // namespace LLGI
