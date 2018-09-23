#include "LLGI.G3.Platform.h"

namespace LLGI
{
namespace G3
{

#if defined(__linux__)
Platform* CreatePlatform(PlatformType platform)
{
	return nullptr;
}
#elif defined(_WIN32)
Platform* CreatePlatform(PlatformType platform)
{
	return nullptr;
}
#endif

void Platform::NewFrame()
{
}

void Platform::Present()
{
}

Graphics* Platform::CreateGraphics()
{
	return nullptr;
}

}
}