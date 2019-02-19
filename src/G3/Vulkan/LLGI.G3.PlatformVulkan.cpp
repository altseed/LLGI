#include "LLGI.G3.PlatformVulkan.h"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace LLGI
{
namespace G3
{

#ifdef _WIN32
LRESULT LLGI_WndProc_Vulkan(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

void PlatformDX12::Wait()
{
	if (fence == nullptr)
		return;

	HRESULT hr;

	auto fenceValue_ = fenceValue;
	hr = commandQueue->Signal(fence, fenceValue_);
	if (FAILED(hr))
	{
		return;
	}

	fenceValue++;

	if (fence->GetCompletedValue() < fenceValue_)
	{
		hr = fence->SetEventOnCompletion(fenceValue_, fenceEvent);
		if (FAILED(hr))
		{
			return;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

#ifdef _DEBUG
VkBool32 PlatformVulkan::DebugMessageCallback(VkDebugReportFlagsEXT flags,
											  VkDebugReportObjectTypeEXT objType,
											  uint64_t srcObject,
											  size_t location,
											  int32_t msgCode,
											  const char* pLayerPrefix,
											  const char* pMsg,
											  void* pUserData)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		std::cout << "ERROR: ";
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		std::cout << "WARNING: ";
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		std::cout << "PERF: ";
	}
	else
	{
		return false;
	}
	std::cout << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << std::endl;

	return false;
}
#endif

bool PlatformVulkan::CreateSwapChain(Vec2I windowSize, bool isVSyncEnabled)
{

	auto oldSwapChain = swapchain;

	frameIndex = 0;

	auto caps = vkPhysicalDevice.getSurfaceCapabilitiesKHR(surface);
	vk::Extent2D swapchainExtent(windowSize.X, windowSize.Y);
	if (caps.currentExtent.width > -1 && caps.currentExtent.height > -1)
	{
		swapchainExtent = caps.currentExtent;
	}

	// select sync or vsync
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
	if (!isVSyncEnabled)
	{
		for (auto mode : vkPhysicalDevice.getSurfacePresentModesKHR(surface))

		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				swapchainPresentMode = vk::PresentModeKHR::eMailbox;
				break;
			}
			else if (mode == vk::PresentModeKHR::eImmediate)
			{
				swapchainPresentMode = vk::PresentModeKHR::eImmediate;
			}
		}
	}

	// decide the number of swapchain
	swapBufferCount = caps.minImageCount + 1;
	if ((caps.maxImageCount > 0) && (swapBufferCount > caps.maxImageCount))
	{
		swapBufferCount = caps.maxImageCount;
	}

	// deside a transform
	vk::SurfaceTransformFlagBitsKHR preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	if (!(caps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity))
	{
		preTransform = caps.currentTransform;
	}

	// create swapchain
	vk::SwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapBufferCount;
	swapchainCreateInfo.imageFormat = surfaceFormat;
	swapchainCreateInfo.imageColorSpace = surfaceColorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapchainCreateInfo.preTransform = preTransform;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.oldSwapchain = oldSwapChain;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	swapchain = vkDevice.createSwapchainKHR(swapchainCreateInfo);
	if (!swapchain)
	{
		return false;
	}

	// remove old swap chain
	if (oldSwapChain)
	{
		for (uint32_t i = 0; i < swapBufferCount; i++)
		{
			g_VkDevice.destroyImageView(m_images[i].view);
		}
		g_VkDevice.destroySwapchainKHR(oldSwapchain);
	}

	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.format = surfaceFormat;
	viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;

	auto swapChainImages = vkDevice.getSwapchainImagesKHR(swapchain);
	m_imageCount = static_cast<uint32_t>(swapChainImages.size());

	m_images.resize(m_imageCount);
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		m_images[i].image = swapChainImages[i];
		viewCreateInfo.image = swapChainImages[i];
		m_images[i].view = vkDevice.createImageView(viewCreateInfo);
		m_images[i].fence = vk::Fence();
	}

	return true;
}

PlatformVulkan::PlatformVulkan() {}

PlatformVulkan::~PlatformVulkan()
{
	Wait();

	SafeRelease(descriptorHeapRTV);

	for (int32_t i = 0; i < SwapBufferCount; i++)
	{
		SafeRelease(RenderPass[i]);
	}

	SafeRelease(commandAllocator);
	SafeRelease(commandListStart);
	SafeRelease(commandListPresent);
	SafeRelease(commandQueue);
	SafeRelease(fence);
	SafeRelease(device);
	SafeRelease(dxgiFactory);
	SafeRelease(swapChain);

	if (fenceEvent != nullptr)
	{
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	// destroy vulkan

	if (vkQueue)
	{
		vkQueue.waitIdle();
	}

	if (vkDevice)
	{
		vkDevice.waitIdle();
		vkDevice.destroyCommandPool(vkCmdPool);
		vkDevice.destroyPipelineCache(vkPipelineCache);
		vkDevice.destroy();
	}

#if defined(_DEBUG)
	if (vkInstance)
	{
		destroyDebugReportCallback((VkInstance)vkInstance, debugReportCallback, nullptr);
	}
#endif
	if (vkInstance)
	{
		vkInstance.destroy();
		vkInstance = nullptr;
	}

	// destroy windows
#ifdef _WIN32
	DestroyWindow(hwnd);
	UnregisterClassA("Vulkan", GetModuleHandle(NULL));
#endif
}

bool PlatformVulkan::Initialize(Vec2I windowSize)
{
#ifdef _WIN32
	// Windows
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = (WNDPROC)LLGI_WndProc_Vulkan;
	wcex.lpszClassName = "Vulkan";
	wcex.hInstance = GetModuleHandle(NULL);
	hInstance = wcex.hInstance;
	RegisterClassExA(&wcex);

	hwnd = CreateWindowA("Vulkan", "Vulkan", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, NULL, NULL, wcex.hInstance, NULL);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
#endif

	// initialize Vulkan context

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "Vulkan";
	appInfo.pEngineName = "Vulkan";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// specify extension
	const std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};

	// create instance
	vk::InstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
#if defined(_DEBUG)
	const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

	instanceCreateInfo.enabledLayerCount = validationLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

	vkInstance = vk::createInstance(instanceCreateInfo);
	if (!vkInstance)
	{
		goto FAILED_EXIT;
	}

	// get physics device
	vkPhysicalDevice = vkInstance.enumeratePhysicalDevices()[0];

	struct Version
	{
		uint32_t patch : 12;
		uint32_t minor : 10;
		uint32_t major : 10;
	} _version;

	vk::PhysicalDeviceProperties deviceProperties = vkPhysicalDevice.getProperties();
	memcpy(&_version, &deviceProperties.apiVersion, sizeof(uint32_t));
	vk::PhysicalDeviceFeatures deviceFeatures = vkPhysicalDevice.getFeatures();
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = vkPhysicalDevice.getMemoryProperties();

	// create device

	// find queue for graphics
	int32_t graphicsQueueInd = -1;
	for (size_t i = 0; i < vkPhysicalDevice.getQueueFamilyProperties().size(); i++)
	{
		auto& queueProp = vkPhysicalDevice.getQueueFamilyProperties()[i];
		if (queueProp.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsQueueInd = i;
			break;
		}
	}

	if (graphicsQueueInd < 0)
	{
		goto FAILED_EXIT;
	}

	float queuePriorities[] = {0.0f};
	vk::DeviceQueueCreateInfo queueCreateInfo;
	queueCreateInfo.queueFamilyIndex = graphicsQueueInd;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities;

	auto deviceFeatures = vkPhysicalDevice.getFeatures();

	const std::vector<const char*> enabledExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(_DEBUG)
	// VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
	};
	vk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	deviceCreateInfo.enabledLayerCount = validationLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();

	vkDevice = vkPhysicalDevice.createDevice(deviceCreateInfo);

	if (!vkDevice)
	{
		goto FAILED_EXIT;
	}

#if defined(_DEBUG)
	// get callbacks
	createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkInstance.getProcAddr("vkCreateDebugReportCallbackEXT");
	destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkInstance.getProcAddr("vkDestroyDebugReportCallbackEXT");
	debugReportMessage = (PFN_vkDebugReportMessageEXT)vkInstance.getProcAddr("vkDebugReportMessageEXT");

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	vk::DebugReportFlagsEXT flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
	dbgCreateInfo.flags = flags.operator VkSubpassDescriptionFlags();

	VkResult err = createDebugReportCallback((VkInstance)vkInstance, &dbgCreateInfo, nullptr, &debugReportCallback);
	if (err)
	{
		goto FAILED_EXIT;
	}
#endif

	vkPipelineCache = vkDevice.createPipelineCache(vk::PipelineCacheCreateInfo());

	vkQueue = vkDevice.getQueue(graphicsQueueInd, 0);

	// create command pool
	vk::CommandPoolCreateInfo cmdPoolInfo;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueInd;
	cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	vkCmdPool = vkDevice.createCommandPool(cmdPoolInfo);

	// create surface
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = hwnd;
	surface = vkInstance.createWin32SurfaceKHR(surfaceCreateInfo);

	if (!surface)
	{
		goto FAILED_EXIT;
	}

	// get supported formats
	auto surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(surface);
	surfaceFormat = vk::Format::eR8G8B8A8Unorm;
	if (surfaceFormats[0].format != vk::Format::eUndefined)
	{
		surfaceFormat = surfaceFormats[0].format;
	}

	surfaceColorSpace = surfaceFormats[0].colorSpace;

	// create swapchain
	assert(0);

	return true;

FAILED_EXIT:;

	if (vkInstance)
	{
		vkInstance.destroy();
		vkInstance = nullptr;
	}

	if (vkDevice)
	{
		vkDevice.destroy();
		vkDevice = nullptr;
	}

	return false;
}

void PlatformVulkan::NewFrame()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		else
		{
			break;
		}
	}

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	commandListStart->Reset(commandAllocator, nullptr);

	D3D12_RESOURCE_BARRIER barrier;
	ZeroMemory(&barrier, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = RenderPass[frameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandListStart->ResourceBarrier(1, &barrier);
	commandListStart->OMSetRenderTargets(1, &(handleRTV[frameIndex]), FALSE, nullptr);
	commandListStart->Close();

	ID3D12CommandList* commandList[] = {commandListStart};
	commandQueue->ExecuteCommandLists(1, commandList);
}

void PlatformVulkan::Present()
{
	commandListPresent->Reset(commandAllocator, nullptr);

	D3D12_RESOURCE_BARRIER barrier;
	ZeroMemory(&barrier, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = RenderPass[frameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	commandListPresent->ResourceBarrier(1, &barrier);
	commandListPresent->Close();

	ID3D12CommandList* commandList[] = {commandListPresent};
	commandQueue->ExecuteCommandLists(1, commandList);

	swapChain->Present(1, 0);
	Wait();
}

Graphics* PlatformVulkan::CreateGraphics()
{
	std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc =
		[this]() -> std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*> {
		std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*> ret(handleRTV[frameIndex], RenderPass[frameIndex]);

		return ret;
	};

	auto graphics = new GraphicsDX12(device, getScreenFunc, commandQueue);

	graphics->SetWindowSize(Vec2I(1280, 720));

	return graphics;
}

} // namespace G3
} // namespace LLGI
