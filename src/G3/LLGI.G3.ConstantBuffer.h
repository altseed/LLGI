
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

	class ConstantBuffer
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

}
}