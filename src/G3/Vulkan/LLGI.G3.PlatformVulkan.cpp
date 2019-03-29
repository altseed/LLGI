
#include "LLGI.G3.PlatformVulkan.h"
#include "LLGI.G3.GraphicsVulkan.h"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace LLGI
{
namespace G3
{

uint32_t GetMemoryTypeIndex(uint32_t bits, const vk::MemoryPropertyFlags& properties, vk::PhysicalDevice vkPhysicalDevice)
{
	uint32_t result = 0;
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = vkPhysicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((bits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		bits >>= 1;
	}

	assert(!"NOT found memory type.\n");
	return 0xffffffff;
}

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
	if (static_cast<int32_t>(caps.currentExtent.width) > -1 && static_cast<int32_t>(caps.currentExtent.height) > -1)
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
	auto desiredSwapBufferCount = caps.minImageCount + 1;
	if ((caps.maxImageCount > 0) && (desiredSwapBufferCount > caps.maxImageCount))
	{
		desiredSwapBufferCount = caps.maxImageCount;
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
	swapchainCreateInfo.minImageCount = desiredSwapBufferCount;
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

	// remove old swap chain
	if (oldSwapChain)
	{
		for (uint32_t i = 0; i < swapBuffers.size(); i++)
		{
			vkDevice.destroyImageView(swapBuffers[i].view);
		}
		vkDevice.destroySwapchainKHR(oldSwapChain);
	}

	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.format = surfaceFormat;
	viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;

	auto swapChainImages = vkDevice.getSwapchainImagesKHR(swapchain);
	swapBufferCount = static_cast<uint32_t>(swapChainImages.size());

	swapBuffers.resize(swapBufferCount);
	for (uint32_t i = 0; i < swapBuffers.size(); i++)
	{
		swapBuffers[i].image = swapChainImages[i];
		viewCreateInfo.image = swapChainImages[i];
		swapBuffers[i].view = vkDevice.createImageView(viewCreateInfo);
		swapBuffers[i].fence = vk::Fence();
	}

	return true;
}

uint32_t PlatformVulkan::AcquireNextImage(vk::Semaphore& semaphore)
{
	auto resultValue = vkDevice.acquireNextImageKHR(swapchain, UINT64_MAX, semaphore, vk::Fence());
	assert(resultValue.result == vk::Result::eSuccess);

	frameIndex = resultValue.value;
	return frameIndex;
}

vk::Result PlatformVulkan::Present(vk::Semaphore semaphore)
{
	vk::PresentInfoKHR presentInfo;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &frameIndex;
	presentInfo.waitSemaphoreCount = semaphore ? 1 : 0;
	presentInfo.pWaitSemaphores = &semaphore;
	return vkQueue.presentKHR(presentInfo);
}

void PlatformVulkan::SetImageLayout(vk::CommandBuffer cmdbuffer,
									vk::Image image,
									vk::ImageLayout oldImageLayout,
									vk::ImageLayout newImageLayout,
									vk::ImageSubresourceRange subresourceRange)
{
	// setup image barrior object
	vk::ImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// current layout
	if (oldImageLayout == vk::ImageLayout::ePreinitialized)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (oldImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	else if (oldImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;

	// next layout
	if (newImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (newImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead; // | imageMemoryBarrier.srcAccessMask;
	else if (newImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	// Put barrier on top
	// Put barrier inside setup command buffer
	cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
							  vk::PipelineStageFlagBits::eTopOfPipe,
							  vk::DependencyFlags(),
							  nullptr,
							  nullptr,
							  imageMemoryBarrier);
}

void PlatformVulkan::Reset()
{
	if (vkInstance != nullptr)
	{
		if (surface != nullptr)
		{
			vkInstance.destroySurfaceKHR(surface);
			surface = nullptr;
		}
	}

	if (vkDevice != nullptr)
	{
		for (auto& swapBuffer : swapBuffers)
		{
			if (swapBuffer.view != nullptr)
			{
				vkDevice.destroyImageView(swapBuffer.view);
			}

			if (swapBuffer.fence != nullptr)
			{
				vkDevice.destroyFence(swapBuffer.fence);
			}
		}
		swapBuffers.clear();

		if (swapchain != nullptr)
		{
			vkDevice.destroySwapchainKHR(swapchain);
			swapchain = nullptr;
		}

		if (vkPipelineCache != nullptr)
		{
			vkDevice.destroyPipelineCache(vkPipelineCache);
			vkPipelineCache = nullptr;
		}

		if (vkPresentComplete != nullptr)
		{
			vkDevice.destroySemaphore(vkPresentComplete);
			vkPresentComplete = nullptr;
		}

		if (vkRenderComplete != nullptr)
		{
			vkDevice.destroySemaphore(vkRenderComplete);
			vkRenderComplete = nullptr;
		}

		if (vkCmdPool != nullptr)
		{
			vkDevice.destroyCommandPool(vkCmdPool);
			vkCmdPool = nullptr;
		}
	}

	if (vkQueue)
	{
		vkQueue = nullptr;
	}
}

PlatformVulkan::PlatformVulkan() {}

PlatformVulkan::~PlatformVulkan()
{
	// destroy vulkan

	// wait
	if (vkQueue)
	{
		vkQueue.waitIdle();
	}

	if (vkDevice)
	{
		vkDevice.waitIdle();
	}

	Reset();

	if (depthStencilBuffer.image != nullptr)
	{
		vkDevice.destroyImageView(depthStencilBuffer.view);
		vkDevice.destroyImage(depthStencilBuffer.image);
		vkDevice.freeMemory(depthStencilBuffer.devMem);

		depthStencilBuffer.image = nullptr;
		depthStencilBuffer.view = nullptr;
	}

	if (vkDevice)
	{
		vkDevice.destroy();
		vkDevice = nullptr;
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

	auto exitWithError = [this]() -> void {
		Reset();

		if (vkDevice)
		{
			vkDevice.destroy();
			vkDevice = nullptr;
		}

		if (vkInstance)
		{
			vkInstance.destroy();
			vkInstance = nullptr;
		}
	};

	try
	{
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
			exitWithError();
			return false;
		}

		float queuePriorities[] = {0.0f};
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.queueFamilyIndex = graphicsQueueInd;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriorities;

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
			exitWithError();
			return false;
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

		// get supported formats
		auto surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(surface);

		surfaceFormat = vk::Format::eR8G8B8A8Unorm;
		if (surfaceFormats[0].format != vk::Format::eUndefined)
		{
			surfaceFormat = surfaceFormats[0].format;
		}

		surfaceColorSpace = surfaceFormats[0].colorSpace;

		// create swapchain
		if (!CreateSwapChain(windowSize, true))
		{
			exitWithError();
			return false;
		}

		// create semaphore
		vk::SemaphoreCreateInfo semaphoreCreateInfo;

		vkPresentComplete = vkDevice.createSemaphore(semaphoreCreateInfo);

		vkRenderComplete = vkDevice.createSemaphore(semaphoreCreateInfo);

		// create command buffer
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = vkCmdPool;
		allocInfo.commandBufferCount = swapBufferCount;
		vkCmdBuffers = vkDevice.allocateCommandBuffers(allocInfo);

		// create depth buffer
		{
			// check a format whether specified format is supported
			vk::Format depthFormat = vk::Format::eD32SfloatS8Uint;
			vk::FormatProperties formatProps = vkPhysicalDevice.getFormatProperties(depthFormat);
			assert(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment);

			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

			// create an image
			vk::ImageCreateInfo imageCreateInfo;
			imageCreateInfo.imageType = vk::ImageType::e2D;
			imageCreateInfo.extent = vk::Extent3D(windowSize.X, windowSize.Y, 1);
			imageCreateInfo.format = depthFormat;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			depthStencilBuffer.image = vkDevice.createImage(imageCreateInfo);

			// allocate memory
			vk::MemoryRequirements memReqs = vkDevice.getImageMemoryRequirements(depthStencilBuffer.image);
			vk::MemoryAllocateInfo memAlloc;
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex =
				GetMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, vkPhysicalDevice);
			depthStencilBuffer.devMem = vkDevice.allocateMemory(memAlloc);
			vkDevice.bindImageMemory(depthStencilBuffer.image, depthStencilBuffer.devMem, 0);

			// create view
			vk::ImageViewCreateInfo viewCreateInfo;
			viewCreateInfo.viewType = vk::ImageViewType::e2D;
			viewCreateInfo.format = depthFormat;
			viewCreateInfo.components = {
				vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
			viewCreateInfo.subresourceRange.aspectMask = aspect;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.layerCount = 1;
			viewCreateInfo.image = depthStencilBuffer.image;
			depthStencilBuffer.view = vkDevice.createImageView(viewCreateInfo);

			// change layout
			{
				vk::CommandBufferBeginInfo cmdBufferBeginInfo;
				vk::BufferCopy copyRegion;

				// start to store commands
				vkCmdBuffers[0].begin(cmdBufferBeginInfo);

				vk::ImageSubresourceRange subresourceRange;
				subresourceRange.aspectMask = aspect;
				subresourceRange.levelCount = 1;
				subresourceRange.layerCount = 1;
				SetImageLayout(vkCmdBuffers[0],
							   depthStencilBuffer.image,
							   vk::ImageLayout::eUndefined,
							   vk::ImageLayout::eDepthStencilAttachmentOptimal,
							   subresourceRange);

				vkCmdBuffers[0].end();

				// submit and wait
				vk::SubmitInfo copySubmitInfo;
				copySubmitInfo.commandBufferCount = 1;
				copySubmitInfo.pCommandBuffers = &vkCmdBuffers[0];

				vkQueue.submit(copySubmitInfo, VK_NULL_HANDLE);
				vkQueue.waitIdle();
			}
		}

		windowSize_ = windowSize;

		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Initialize Failed : " << e.what() << std::endl;
		std::cout << "Please install Vulkan client driver." << std::endl;
		exitWithError();
		return false;
	}
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

	AcquireNextImage(vkPresentComplete);

	// reset commands
	auto& cmdBuffer = vkCmdBuffers[frameIndex];

	cmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	vk::CommandBufferBeginInfo cmdBufInfo;
	cmdBuffer.begin(cmdBufInfo);
}

void PlatformVulkan::Present() { Present(vkRenderComplete); }

Graphics* PlatformVulkan::CreateGraphics()
{
	PlatformView platformView;

	for (size_t i = 0; i < swapBuffers.size(); i++)
	{
		platformView.colors.push_back(swapBuffers[i].image);
		platformView.colorViews.push_back(swapBuffers[i].view);
		platformView.depths.push_back(depthStencilBuffer.image);
		platformView.depthViews.push_back(depthStencilBuffer.view);
	}

	platformView.imageSize = windowSize_;
	platformView.format = surfaceFormat;

	auto getStatus = [this](PlatformStatus& status) -> void { status.currentSwapBufferIndex = this->frameIndex; };
	auto graphics = new GraphicsVulkan(vkDevice, vkQueue, vkCmdPool, vkPhysicalDevice, platformView, getStatus);

	return graphics;
}

} // namespace G3
} // namespace LLGI
