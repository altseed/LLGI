
#include "../LLGI.G3.Compiler.h"
#include "../LLGI.G3.Platform.h"

#ifdef _WIN32
#include "../DX12/LLGI.G3.CompilerDX12.h"
#include "../DX12/LLGI.G3.PlatformDX12.h"
#endif

#ifdef __APPLE__
#include "../Metal/LLGI.G3.CompilerMetal.h"
#include "../Metal/LLGI.G3.PlatformMetal.h"
#endif

namespace LLGI
{
namespace G3
{

Platform* CreatePlatform(DeviceType platform)
{
#ifdef _WIN32
	Vec2I windowSize;
	windowSize.X = 1280;
	windowSize.Y = 720;
	auto obj = new PlatformDX12();
	if (!obj->Initialize(windowSize))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
#endif

#ifdef __APPLE__
	auto obj = new PlatformMetal();
	return obj;
#endif
}

Compiler* CreateCompiler(DeviceType device)
{
#ifdef _WIN32
	auto obj = new CompilerDX12();
	return obj;
#endif

#ifdef __APPLE__
	return nullptr;
#endif
}

} // namespace G3
} // namespace LLGI
