#ifdef _WIN32
#define NOMINMAX
#endif

#include "LLGI.PlatformVulkan.h"
#include "LLGI.GraphicsVulkan.h"
#include "LLGI.TextureVulkan.h"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <X11/Xlib-xcb.h>
#undef Always
#endif

namespace LLGI
{

#if !defined(NDEBUG)
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

bool PlatformVulkan::CreateSwapChain(Vec2I windowSize, bool waitVSync)
{
	auto oldSwapChain = swapchain_;

	frameIndex = 0;

	auto caps = vkPhysicalDevice.getSurfaceCapabilitiesKHR(surface_);
	vk::Extent2D swapchainExtent(windowSize.X, windowSize.Y);
	if (caps.currentExtent.width != 0xFFFFFFFF)
	{
		swapchainExtent = caps.currentExtent;
	}

	// select sync or vsync
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
	if (!waitVSync)
	{
		for (auto mode : vkPhysicalDevice.getSurfacePresentModesKHR(surface_))

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
	swapchainCreateInfo.surface = surface_;
	swapchainCreateInfo.minImageCount = desiredSwapBufferCount;
	swapchainCreateInfo.imageFormat = surfaceFormat;
	swapchainCreateInfo.imageColorSpace = surfaceColorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst |
									 vk::ImageUsageFlagBits::eTransferSrc; // eTransferSrc: for capture
	swapchainCreateInfo.preTransform = preTransform;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.oldSwapchain = oldSwapChain;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	swapBufferCountMin_ = desiredSwapBufferCount;

	swapchain_ = vkDevice_.createSwapchainKHR(swapchainCreateInfo);

	// remove old swap chain
	if (oldSwapChain)
	{
		for (uint32_t i = 0; i < swapBuffers.size(); i++)
		{
			vkDevice_.destroyImageView(swapBuffers[i].view);
			SafeRelease(swapBuffers[i].texture);
		}
		vkDevice_.destroySwapchainKHR(oldSwapChain);
	}

	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.format = surfaceFormat;
	viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;

	auto swapChainImages = vkDevice_.getSwapchainImagesKHR(swapchain_);
	swapBufferCount = static_cast<uint32_t>(swapChainImages.size());

	swapBuffers.resize(swapBufferCount);
	for (uint32_t i = 0; i < swapBuffers.size(); i++)
	{
		swapBuffers[i].image = swapChainImages[i];
		viewCreateInfo.image = swapChainImages[i];
		swapBuffers[i].view = vkDevice_.createImageView(viewCreateInfo);
		swapBuffers[i].fence = vk::Fence();

		swapBuffers[i].texture = new TextureVulkan();
		if (!swapBuffers[i].texture->InitializeAsScreen(swapBuffers[i].image, swapBuffers[i].view, surfaceFormat, windowSize))
		{
			Log(LogType::Error, "failed to create a texture while creating swap buffers.");
			throw "failed to create a texture while creating swap buffers.";
		}
	}

	return true;
}

bool PlatformVulkan::CreateDepthBuffer(Vec2I windowSize)
{
	SafeRelease(depthStencilTexture_);

	depthStencilTexture_ = new TextureVulkan();
	if (!depthStencilTexture_->InitializeAsDepthStencil(vkDevice_, vkPhysicalDevice, windowSize, nullptr))
	{
		return false;
	}
	return true;
}

void PlatformVulkan::CreateRenderPass()
{
	renderPasses.clear();
	for (size_t i = 0; i < swapBuffers.size(); i++)
	{
		auto renderPass = new RenderPassVulkan(renderPassPipelineStateCache_, vkDevice_, nullptr);

		std::array<TextureVulkan*, 1> textures;
		textures[0] = swapBuffers[i].texture;

		renderPass->Initialize(const_cast<const TextureVulkan**>(textures.data()), 1, depthStencilTexture_);

		renderPasses.emplace_back(CreateSharedPtr(renderPass));
	}
}

uint32_t PlatformVulkan::AcquireNextImage(vk::Semaphore& semaphore)
{
	auto resultValue = vkDevice_.acquireNextImageKHR(swapchain_, UINT64_MAX, semaphore, vk::Fence());
	assert(resultValue.result == vk::Result::eSuccess);

	frameIndex = resultValue.value;
	return frameIndex;
}

vk::Fence PlatformVulkan::GetSubmitFence(bool destroy)
{
	auto& image = swapBuffers[frameIndex];
	while (image.fence)
	{
		vk::Result fenceRes = vkDevice_.waitForFences(image.fence, VK_TRUE, std::numeric_limits<int>::max());
		if (fenceRes == vk::Result::eSuccess)
		{
			if (destroy)
			{
				vkDevice_.destroyFence(image.fence);
			}
			image.fence = vk::Fence();
		}
	}

	image.fence = vkDevice_.createFence(vk::FenceCreateFlags());
	return image.fence;
}

vk::Result PlatformVulkan::Present(vk::Semaphore semaphore)
{
	vk::PresentInfoKHR presentInfo;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain_;
	presentInfo.pImageIndices = &frameIndex;
	presentInfo.waitSemaphoreCount = semaphore ? 1 : 0;
	presentInfo.pWaitSemaphores = &semaphore;
	return vkQueue.presentKHR(presentInfo);
}

/*
void PlatformVulkan::SetImageBarrior(vk::CommandBuffer cmdbuffer,
									 vk::Image image,
									 vk::ImageLayout oldImageLayout,
									 vk::ImageLayout newImageLayout,
									 vk::ImageSubresourceRange subresourceRange)
{
	assert(newImageLayout != vk::ImageLayout::eUndefined);
	assert(newImageLayout != vk::ImageLayout::ePreinitialized);

	// setup image barrior object
	vk::ImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// current layout
	if (oldImageLayout == vk::ImageLayout::ePreinitialized)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
	else if (oldImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	else if (oldImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
	else if (oldImageLayout == vk::ImageLayout::ePresentSrcKHR)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;

	// next layout
	if (newImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	else if (newImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	else if (newImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;

	// Put barrier on top
	// Put barrier inside setup command buffer
	cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
							  vk::PipelineStageFlagBits::eAllCommands,
							  vk::DependencyFlags(),
							  nullptr,
							  nullptr,
							  imageMemoryBarrier);
}
*/

void PlatformVulkan::Reset()
{
	if (vkDevice_)
	{
		for (auto& swapBuffer : swapBuffers)
		{
			if (swapBuffer.view)
			{
				vkDevice_.destroyImageView(swapBuffer.view);
			}

			if (swapBuffer.fence)
			{
				vkDevice_.destroyFence(swapBuffer.fence);
			}

			SafeRelease(swapBuffer.texture);
		}
		swapBuffers.clear();

		if (swapchain_)
		{
			vkDevice_.destroySwapchainKHR(swapchain_);
			swapchain_ = nullptr;
		}

		renderPasses.clear();

		if (vkPipelineCache_)
		{
			vkDevice_.destroyPipelineCache(vkPipelineCache_);
			vkPipelineCache_ = nullptr;
		}

		if (vkPresentComplete_)
		{
			vkDevice_.destroySemaphore(vkPresentComplete_);
			vkPresentComplete_ = nullptr;
		}

		if (vkRenderComplete_)
		{
			vkDevice_.destroySemaphore(vkRenderComplete_);
			vkRenderComplete_ = nullptr;
		}

		if (vkCmdPool_)
		{
			vkDevice_.destroyCommandPool(vkCmdPool_);
			vkCmdPool_ = nullptr;
		}
	}

	if (vkInstance_)
	{
		if (surface_)
		{
			vkInstance_.destroySurfaceKHR(surface_);
			surface_ = nullptr;
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

	if (vkDevice_)
	{
		vkDevice_.waitIdle();
	}

	Reset();

	SafeRelease(depthStencilTexture_);
	/*
	if (depthStencilBuffer.image)
	{
		vkDevice_.destroyImageView(depthStencilBuffer.view);
		vkDevice_.destroyImage(depthStencilBuffer.image);
		vkDevice_.freeMemory(depthStencilBuffer.devMem);

		depthStencilBuffer.image = nullptr;
		depthStencilBuffer.view = nullptr;
	}
	*/

	SafeRelease(renderPassPipelineStateCache_);

	if (vkDevice_)
	{
		vkDevice_.destroy();
		vkDevice_ = nullptr;
	}

#if !defined(NDEBUG)
	if (vkInstance_)
	{
		destroyDebugReportCallback((VkInstance)vkInstance_, debugReportCallback, nullptr);
	}
#endif
	if (vkInstance_)
	{
		vkInstance_.destroy();
		vkInstance_ = nullptr;
	}
}

bool PlatformVulkan::Initialize(Window* window, bool waitVSync)
{
	window_ = window;
	waitVSync_ = waitVSync;

	// initialize Vulkan context

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "Vulkan";
	appInfo.pEngineName = "Vulkan";
	appInfo.apiVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// specify extension
	const std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#if !defined(NDEBUG)
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

	auto exitWithError = [this]() -> void {
		Reset();

		SafeRelease(depthStencilTexture_);

		if (vkDevice_)
		{
			vkDevice_.destroy();
			vkDevice_ = nullptr;
		}

		if (vkInstance_)
		{
			vkInstance_.destroy();
			vkInstance_ = nullptr;
		}
	};

	try
	{
		// create instance
		vk::InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
#if !defined(NDEBUG)

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

		if (layerCount > 0)
		{
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
#endif

		vkInstance_ = vk::createInstance(instanceCreateInfo);

		// get physics device
		auto physicalDevices = vkInstance_.enumeratePhysicalDevices();
		vkPhysicalDevice = physicalDevices[0];

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

		// create surface
#ifdef _WIN32
		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.hinstance = (HINSTANCE)window->GetNativePtr(1);
		surfaceCreateInfo.hwnd = (HWND)window->GetNativePtr(0);
		surface_ = vkInstance_.createWin32SurfaceKHR(surfaceCreateInfo);
#else
		vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
		surfaceCreateInfo.connection = XGetXCBConnection((Display*)window->GetNativePtr(0));
		surfaceCreateInfo.window = ((::Window)window->GetNativePtr(1));
		surface_ = vkInstance_.createXcbSurfaceKHR(surfaceCreateInfo);
#endif
		// create device

		// find queue for graphics
		int32_t graphicsQueueInd = -1;

		auto queueFamilyProperties = vkPhysicalDevice.getQueueFamilyProperties();

		for (size_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			auto& queueProp = queueFamilyProperties[i];
			if (queueProp.queueFlags & vk::QueueFlagBits::eGraphics /* && vkPhysicalDevice.getSurfaceSupportKHR(i, surface_)*/)
			{
				graphicsQueueInd = static_cast<int32_t>(i);
				break;
			}
		}

		if (graphicsQueueInd < 0)
		{
			exitWithError();
			Log(LogType::Warning, "Faile to get graphics queue index.");
			return false;
		}

		float queuePriorities[] = {0.0f};
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.queueFamilyIndex = graphicsQueueInd;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriorities;
		queueFamilyIndex_ = queueCreateInfo.queueFamilyIndex;

		const std::vector<const char*> enabledExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if !defined(NDEBUG)
		// VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
		};
		vk::DeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

#if !defined(NDEBUG)
		if (layerCount > 0)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif
		vkDevice_ = vkPhysicalDevice.createDevice(deviceCreateInfo);

#if !defined(NDEBUG)
		// get callbacks
		createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkInstance_.getProcAddr("vkCreateDebugReportCallbackEXT");
		destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkInstance_.getProcAddr("vkDestroyDebugReportCallbackEXT");
		debugReportMessage = (PFN_vkDebugReportMessageEXT)vkInstance_.getProcAddr("vkDebugReportMessageEXT");

		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
		vk::DebugReportFlagsEXT flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)DebugMessageCallback;
		dbgCreateInfo.flags = flags.operator VkSubpassDescriptionFlags();

		VkResult err = createDebugReportCallback((VkInstance)vkInstance_, &dbgCreateInfo, nullptr, &debugReportCallback);
		if (err)
		{
			exitWithError();
			return false;
		}
#endif

		vkPipelineCache_ = vkDevice_.createPipelineCache(vk::PipelineCacheCreateInfo());

		vkQueue = vkDevice_.getQueue(graphicsQueueInd, 0);

		// create command pool
		vk::CommandPoolCreateInfo cmdPoolInfo;
		cmdPoolInfo.queueFamilyIndex = graphicsQueueInd;
		cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		vkCmdPool_ = vkDevice_.createCommandPool(cmdPoolInfo);

		// get supported formats
		auto surfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(surface_);

		surfaceFormat = vk::Format::eR8G8B8A8Unorm;
		if (surfaceFormats[0].format != vk::Format::eUndefined)
		{
			surfaceFormat = surfaceFormats[0].format;
		}

		surfaceColorSpace = surfaceFormats[0].colorSpace;

		// create swapchain
		if (!vkPhysicalDevice.getSurfaceSupportKHR(graphicsQueueInd, surface_))
		{
		}

		if (!CreateSwapChain(window->GetWindowSize(), waitVSync))
		{
			Log(LogType::Error, "Swapchain is not supported.");
			exitWithError();
			return false;
		}

		// create semaphore
		vk::SemaphoreCreateInfo semaphoreCreateInfo;

		vkPresentComplete_ = vkDevice_.createSemaphore(semaphoreCreateInfo);

		vkRenderComplete_ = vkDevice_.createSemaphore(semaphoreCreateInfo);

		// create command buffer
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.commandPool = vkCmdPool_;
		allocInfo.commandBufferCount = swapBufferCount;
		vkCmdBuffers = vkDevice_.allocateCommandBuffers(allocInfo);

		// create depth buffer
		if (!CreateDepthBuffer(window->GetWindowSize()))
		{
			exitWithError();
			return false;
		}

		/*
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
			depthStencilBuffer.image = vkDevice_.createImage(imageCreateInfo);

			// allocate memory
			vk::MemoryRequirements memReqs = vkDevice_.getImageMemoryRequirements(depthStencilBuffer.image);
			vk::MemoryAllocateInfo memAlloc;
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex =
				GetMemoryTypeIndex(vkPhysicalDevice, memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
			depthStencilBuffer.devMem = vkDevice_.allocateMemory(memAlloc);
			vkDevice_.bindImageMemory(depthStencilBuffer.image, depthStencilBuffer.devMem, 0);

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
			depthStencilBuffer.view = vkDevice_.createImageView(viewCreateInfo);

			// change layout(nt needed?)

			{
				vk::CommandBufferBeginInfo cmdBufferBeginInfo;
				vk::BufferCopy copyRegion;

				// start to store commands
				vkCmdBuffers[0].begin(cmdBufferBeginInfo);

				vk::ImageSubresourceRange subresourceRange;
				subresourceRange.aspectMask = aspect;
				subresourceRange.levelCount = 1;
				subresourceRange.layerCount = 1;
				SetImageBarrior(vkCmdBuffers[0],
								depthStencilBuffer.image,
								vk::ImageLayout::eUndefined,
								vk::ImageLayout::eDepthStencilAttachmentOptimal,
								subresourceRange);

				vkCmdBuffers[0].end();

				// submit and wait
				std::array<vk::SubmitInfo, 1> copySubmitInfos;
				copySubmitInfos[0].commandBufferCount = 1;
				copySubmitInfos[0].pCommandBuffers = &vkCmdBuffers[0];

				vkQueue.submit(copySubmitInfos.size(), copySubmitInfos.data(), vk::Fence());
				vkQueue.waitIdle();
			}
		}
		*/

		windowSize_ = window->GetWindowSize();
		renderPassPipelineStateCache_ = new RenderPassPipelineStateCacheVulkan(vkDevice_, nullptr);

		// create renderpasses
		CreateRenderPass();

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

bool PlatformVulkan::NewFrame()
{
	if (!window_->OnNewFrame())
	{
		return false;
	}

	AcquireNextImage(vkPresentComplete_);
	executedCommandCount = 0;
	return true;
}

void PlatformVulkan::Present()
{

	// waiting or empty command
	auto& cmdBuffer = vkCmdBuffers[frameIndex];

	cmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	vk::CommandBufferBeginInfo cmdBufInfo;
	cmdBuffer.begin(cmdBufInfo);

	// typical driver causes errors without present command
	if (executedCommandCount == 0)
	{
		vk::ClearColorValue clearColor(std::array<float, 4>{0, 0, 0, 0});
		vk::ClearDepthStencilValue clearDepth(1.0f, 0);

		vk::ImageSubresourceRange colorSubRange;
		colorSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		colorSubRange.levelCount = 1;
		colorSubRange.layerCount = 1;

		vk::ImageSubresourceRange depthSubRange;
		depthSubRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		depthSubRange.levelCount = 1;
		depthSubRange.layerCount = 1;

		// to make screen clear
		SetImageLayout(
			cmdBuffer, swapBuffers[frameIndex].image, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, colorSubRange);
		// SetImageLayout(cmdBuffer,
		//			   depthStencilBuffer.image,
		//				vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal,
		//			   vk::ImageLayout::eTransferDstOptimal,
		//			   depthSubRange);

		// cmdBuffer.clearColorImage(swapBuffers[frameIndex].image, vk::ImageLayout::eColorAttachmentOptimal, clearColor, colorSubRange);
		// cmdBuffer.clearDepthStencilImage(depthStencilBuffer.image, vk::ImageLayout::eDepthStencilAttachmentOptimal, clearDepth,
		// depthSubRange);

		SetImageLayout(cmdBuffer,
					   swapBuffers[frameIndex].image,
					   vk::ImageLayout::eColorAttachmentOptimal,
					   vk::ImageLayout::ePresentSrcKHR,
					   colorSubRange);
	}

	cmdBuffer.end();

	{
		vk::PipelineStageFlags pipelineStages = vk::PipelineStageFlagBits::eBottomOfPipe;
		vk::SubmitInfo submitInfo;
		submitInfo.pWaitDstStageMask = &pipelineStages;

		// send semaphore to be need to wait
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &vkPresentComplete_;

		// set command
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		// set a semaphore which notify to finish to execute commands
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vkRenderComplete_;

		vk::Fence fence = GetSubmitFence(true);
		vkQueue.submit(submitInfo, fence);
		vk::Result fenceRes = vkDevice_.waitForFences(fence, VK_TRUE, std::numeric_limits<int>::max());
		assert(fenceRes == vk::Result::eSuccess);
	}

	Present(vkRenderComplete_);
}

void PlatformVulkan::SetWindowSize(const Vec2I& windowSize)
{
	if (windowSize_ == windowSize)
	{
		return;
	}

	vkDevice_.waitIdle();
	CreateSwapChain(windowSize, waitVSync_);

	CreateDepthBuffer(windowSize);

	CreateRenderPass();
	windowSize_ = windowSize;
}

Graphics* PlatformVulkan::CreateGraphics()
{
	auto addCommand = [this](vk::CommandBuffer commandBuffer, vk::Fence fence) -> void {
		std::array<vk::SubmitInfo, 1> copySubmitInfos;
		copySubmitInfos[0].commandBufferCount = 1;
		copySubmitInfos[0].pCommandBuffers = &commandBuffer;
		vkQueue.submit(copySubmitInfos.size(), copySubmitInfos.data(), fence);

		this->executedCommandCount++;
	};

	auto graphics = new GraphicsVulkan(
		vkDevice_, vkQueue, vkCmdPool_, vkPhysicalDevice, swapBuffers.size(), addCommand, renderPassPipelineStateCache_, this);

	return graphics;
}

RenderPass* PlatformVulkan::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	auto currentRenderPass = renderPasses[frameIndex];

	currentRenderPass->SetClearColor(clearColor);
	currentRenderPass->SetIsColorCleared(isColorCleared);
	currentRenderPass->SetIsDepthCleared(isDepthCleared);
	return currentRenderPass.get();
}

} // namespace LLGI
