
#include "../LLGI.G3.Compiler.h"
#include "../LLGI.G3.Platform.h"

#ifdef ENABLE_VULKAN
#include "../../Vulkan/LLGI.PlatformVulkan.h"
#endif

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

Platform* CreatePlatform(DeviceType platformDeviceType)
{
#ifdef _WIN32
	Vec2I windowSize;
	windowSize.X = 1280;
	windowSize.Y = 720;

	if (platformDeviceType == DeviceType::Default || platformDeviceType == DeviceType::DirectX12)
	{
		auto platform = new PlatformDX12();
		if (!platform->Initialize(windowSize))
		{
			SafeRelease(platform);
			return nullptr;
		}
		return platform;
	}

#ifdef ENABLE_VULKAN
	/*
	if (platformDeviceType == DeviceType::Vulkan)
	{
		auto platform = new PlatformVulkan();
		if (!platform->Initialize(windowSize))
		{
			SafeRelease(platform);
			return nullptr;
		}
		return platform;
	}
	*/
#endif

#endif

#ifdef __APPLE__
	auto obj = new PlatformMetal();
	return obj;
#endif

	return nullptr;
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
