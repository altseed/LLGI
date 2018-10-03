
#include "../LLGI.G3.Platform.h"
#include "../LLGI.G3.Compiler.h"

#ifdef _WIN32
#include "../DX12/LLGI.G3.PlatformDX12.h"
#include "../DX12/LLGI.G3.CompilerDX12.h"
#endif

namespace LLGI
{
namespace G3
{

Platform* CreatePlatform(DeviceType platform)
{
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
}

Compiler* CreateCompiler(DeviceType device)
{
	auto obj = new CompilerDX12();

	return obj;
}

}
}