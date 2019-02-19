
#pragma once

#include "../LLGI.G3.Platform.h"
#include "LLGI.G3.BaseVulkan.h"

namespace LLGI
{
namespace G3
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

	int32_t swapBufferCount = 2;

	vk::Instance vkInstance;
	vk::PhysicalDevice vkPhysicalDevice;
	vk::Device vkDevice;
	vk::PipelineCache vkPipelineCache;
	vk::Queue vkQueue;
	vk::CommandPool vkCmdPool;

	vk::SurfaceKHR surface;
	vk::SwapchainKHR swapchain;
	vk::PresentInfoKHR presentInfo;

	vk::Format surfaceFormat;
	vk::ColorSpaceKHR surfaceColorSpace;

#ifdef _WIN32
	HWND hwnd = nullptr;
	HINSTANCE hInstance = nullptr;
#endif

#ifdef _DEBUG
	PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallback = nullptr;
	PFN_vkDebugReportMessageEXT debugReportMessage = nullptr;
	VkDebugReportCallbackEXT debugReportCallback;
#endif

	ID3D12Device* device = nullptr;
	IDXGIFactory4* dxgiFactory = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12Fence* fence = nullptr;
	HANDLE fenceEvent = nullptr;
	IDXGISwapChain3* swapChain = nullptr;

	ID3D12DescriptorHeap* descriptorHeapRTV = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV[SwapBufferCount];
	ID3D12Resource* RenderPass[SwapBufferCount];

	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandListStart = nullptr;
	ID3D12GraphicsCommandList* commandListPresent = nullptr;
	UINT64 fenceValue = 1;

	int32_t frameIndex = 0;

	void Wait();

	bool CreateSwapChain(Vec2I windowSize, bool isVSyncEnabled);

public:
	PlatformVulkan();
	virtual ~PlatformVulkan();

	bool Initialize(Vec2I windowSize);

	void NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;

	DeviceType GetDeviceType() const override { return DeviceType::Vulkan; }
};

} // namespace G3
} // namespace LLGI