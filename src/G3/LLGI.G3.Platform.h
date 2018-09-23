
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

Platform* CreatePlatform(PlatformType platform);

class Platform
	: public ReferenceObject
{
private:
public:
	Platform() = default;
	virtual ~Platform() = default;

	virtual void NewFrame();
	virtual void Present();
	virtual Graphics* CreateGraphics();
};

}
}