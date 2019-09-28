
#include "../LLGI.Compiler.h"
#include "../LLGI.Platform.h"

#ifdef ENABLE_VULKAN
#include "../Vulkan/LLGI.PlatformVulkan.h"
#endif

#ifdef _WIN32
#include "../DX12/LLGI.CompilerDX12.h"
#include "../DX12/LLGI.PlatformDX12.h"
#endif

#ifdef __APPLE__
#include "../Metal/LLGI.CompilerMetal.h"
#include "../Metal/LLGI.PlatformMetal.h"
#endif

namespace LLGI
{

Platform* CreatePlatform(DeviceType platformDeviceType)
{
	Vec2I windowSize;
	windowSize.X = 1280;
	windowSize.Y = 720;

#ifdef ENABLE_VULKAN
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
#endif

#ifdef _WIN32

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

#elif defined(__APPLE__)
	auto obj = new PlatformMetal(windowSize);
	return obj;
#else

#endif



	return nullptr;
}

Compiler* CreateCompiler(DeviceType device)
{
#ifdef _WIN32
	if (device == DeviceType::Default || device == DeviceType::DirectX12)
	{
		auto obj = new CompilerDX12();
		return obj;
	}
#endif

#ifdef ENABLE_VULKAN
	if (device == DeviceType::Vulkan)
	{
		return nullptr;
	}
#endif

#ifdef __APPLE__
	auto obj = new CompilerMetal();
	return obj;
#endif

	return nullptr;
}

} // namespace LLGI
