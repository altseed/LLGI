

#pragma once

#include "../LLGI.Platform.h"
#include "LLGI.BaseVulkan.h"

#ifdef _WIN32
#include "../Win/LLGI.WindowWin.h"
#endif

namespace LLGI
{

class PlatformVulkan : public Platform
{
private:
#ifdef _DEBUG
	static VkBool32 DebugMessageCallback(VkDebugReportFlagsEXT flags,
										 VkDebugReportObjectTypeEXT objType,
										 uint64_t srcObject,
										 size_t location,
										 int32_t msgCode,
										 const char* pLayerPrefix,
										 const char* pMsg,
										 void* pUserData);
#endif

	class SwapBuffer
	{
	public:
		vk::Image image = nullptr;
		vk::ImageView view = nullptr;
		vk::Fence fence = nullptr;
	};

	struct DepthStencilBuffer
	{
		vk::Image image = nullptr;
		vk::ImageView view = nullptr;
		vk::DeviceMemory devMem = nullptr;
	};

	int32_t swapBufferCount = 2;

	vk::Instance vkInstance = nullptr;
	vk::PhysicalDevice vkPhysicalDevice = nullptr;
	vk::Device vkDevice = nullptr;
	vk::PipelineCache vkPipelineCache = nullptr;
	vk::Queue vkQueue = nullptr;
	vk::CommandPool vkCmdPool = nullptr;

	Vec2I windowSize_;

	//! to check to finish present
	vk::Semaphore vkPresentComplete;

	//! to check to finish render
	vk::Semaphore vkRenderComplete;
	std::vector<vk::CommandBuffer> vkCmdBuffers;

	vk::SurfaceKHR surface = nullptr;
	vk::SwapchainKHR swapchain = nullptr;
	vk::PresentInfoKHR presentInfo;

	vk::Format surfaceFormat;
	vk::ColorSpaceKHR surfaceColorSpace;

	//! depth buffer
	DepthStencilBuffer depthStencilBuffer;

	std::vector<SwapBuffer> swapBuffers;

	int32_t executedCommandCount = 0;

#ifdef _WIN32
	std::shared_ptr<WindowWin> window = nullptr;
#endif

#ifdef _DEBUG
	PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallback = nullptr;
	PFN_vkDebugReportMessageEXT debugReportMessage = nullptr;
	VkDebugReportCallbackEXT debugReportCallback;
#endif
	uint32_t frameIndex = 0;

	bool CreateSwapChain(Vec2I windowSize, bool isVSyncEnabled);

	/*!
		@brief	get swap buffer index
		@param	semaphore	the signaling semaphore to be waited for other functions
	*/
	uint32_t AcquireNextImage(vk::Semaphore& semaphore);

	vk::Fence GetSubmitFence(bool destroy = false);

	/**
		@brief	the semaphore to wait for before present
	*/
	vk::Result Present(vk::Semaphore semaphore);

	void SetImageBarrior(vk::CommandBuffer cmdbuffer,
						vk::Image image,
						vk::ImageLayout oldImageLayout,
						vk::ImageLayout newImageLayout,
						vk::ImageSubresourceRange subresourceRange);

	void Reset();

public:
	PlatformVulkan();
	virtual ~PlatformVulkan();

	bool Initialize(Vec2I windowSize);

	bool NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;

	DeviceType GetDeviceType() const override { return DeviceType::Vulkan; }
};

} // namespace LLGI