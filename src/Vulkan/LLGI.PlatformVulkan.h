

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
		TextureVulkan* texture = nullptr;
	};

	struct DepthStencilBuffer
	{
		vk::Image image = nullptr;
		vk::ImageView view = nullptr;
		vk::DeviceMemory devMem = nullptr;
	};

	int32_t swapBufferCount = 2;

	vk::Instance vkInstance_ = nullptr;
	vk::PhysicalDevice vkPhysicalDevice = nullptr;
	vk::Device vkDevice_ = nullptr;
	vk::PipelineCache vkPipelineCache_ = nullptr;
	vk::Queue vkQueue = nullptr;
	vk::CommandPool vkCmdPool_ = nullptr;

	Vec2I windowSize_;

	//! to check to finish present
	vk::Semaphore vkPresentComplete_;

	//! to check to finish render
	vk::Semaphore vkRenderComplete_;
	std::vector<vk::CommandBuffer> vkCmdBuffers;

	vk::SurfaceKHR surface_ = nullptr;
	vk::SwapchainKHR swapchain_ = nullptr;
	vk::PresentInfoKHR presentInfo;

	vk::Format surfaceFormat;
	vk::ColorSpaceKHR surfaceColorSpace;

	//! depth buffer
	//DepthStencilBuffer depthStencilBuffer;
	TextureVulkan* depthStencilTexture_ = nullptr;

	RenderPassPipelineStateCacheVulkan* renderPassPipelineStateCache_ = nullptr;

	std::vector<std::shared_ptr<RenderPassVulkan>> renderPasses;

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

	RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;

	DeviceType GetDeviceType() const override { return DeviceType::Vulkan; }
};

} // namespace LLGI