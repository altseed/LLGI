
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class Texture
	: public ReferenceObject
{
private:
public:
	Texture() = default;
	virtual ~Texture() = default;

	virtual void* Lock();
	virtual void Unlock();
};

}
}