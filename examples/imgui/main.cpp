
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#endif

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA 1
#endif

#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11 1
#undef Always
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef __linux__
#undef Always
#endif

#include <LLGI.CommandList.h>
#include <LLGI.Graphics.h>
#include <LLGI.Platform.h>

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")
#include <DX12/LLGI.CommandListDX12.h>
#include <DX12/LLGI.GraphicsDX12.h>
#elif __APPLE__
#include <Metal/LLGI.CommandListMetal.h>
#include <Metal/LLGI.GraphicsMetal.h>
#include <Metal/LLGI.Metal_Impl.h>
#else
#include <Vulkan/LLGI.CommandListVulkan.h>
#include <Vulkan/LLGI.GraphicsVulkan.h>
#include <Vulkan/LLGI.RenderPassVulkan.h>
#endif

#include "../thirdparty/imgui/imgui.h"

#ifdef _WIN32
#include "../thirdparty/imgui/imgui_impl_dx12.h"
#elif __APPLE__
#include "../thirdparty/imgui/imgui_impl_metal.h"
#else
#include "../thirdparty/imgui/imgui_impl_vulkan.h"
#endif

#include "../thirdparty/imgui/imgui_impl_glfw.h"

class LLGIWindow : public LLGI::Window
{
	GLFWwindow* window_ = nullptr;

public:
	LLGIWindow(GLFWwindow* window) : window_(window) {}

	bool OnNewFrame() override { return glfwWindowShouldClose(window_) == GL_FALSE; }

	void* GetNativePtr(int32_t index) override
	{
#ifdef _WIN32
		if (index == 0)
		{
			return glfwGetWin32Window(window_);
		}

		return (HINSTANCE)GetModuleHandle(0);
#endif

#ifdef __APPLE__
		return glfwGetCocoaWindow(window_);
#endif

#ifdef __linux__
		if (index == 0)
		{
			return glfwGetX11Display();
		}

		return reinterpret_cast<void*>(glfwGetX11Window(window_));
#endif
	}

	LLGI::Vec2I GetWindowSize() const override
	{
		int w, h;
		glfwGetWindowSize(window_, &w, &h);
		return LLGI::Vec2I(w, h);
	}
};

static void glfw_error_callback(int error, const char* description) { fprintf(stderr, "Glfw Error %d: %s\n", error, description); }

int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto window = glfwCreateWindow(1280, 720, "Example imgui", nullptr, nullptr);

	auto llgiwindow = new LLGIWindow(window);

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default, llgiwindow);
	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);
	auto commandList = graphics->CreateCommandList(sfMemoryPool);

#ifdef _WIN32
	ID3D12DescriptorHeap* srvDescHeap = nullptr;

	auto g = static_cast<LLGI::GraphicsDX12*>(graphics);

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (g->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvDescHeap)) != S_OK)
			throw "Failed to initialize.";
	}
#elif __APPLE__
	auto g = static_cast<LLGI::GraphicsMetal*>(graphics);
#else
	auto g = static_cast<LLGI::GraphicsVulkan*>(graphics);

	/*
		vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = size * stage;

	descriptorPool_ = graphics_->GetDevice().createDescriptorPool(poolInfo);
	*/
#endif

	LLGI::Color8 color;
	color.R = 50;
	color.G = 50;
	color.B = 50;
	color.A = 255;

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window, true);

#ifdef _WIN32
	ImGui_ImplDX12_Init(g->GetDevice(),
						g->GetSwapBufferCount(),
						DXGI_FORMAT_R8G8B8A8_UNORM,
						srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
						srvDescHeap->GetGPUDescriptorHandleForHeapStart());
#elif __APPLE__
	ImGui_ImplMetal_Init(g->GetImpl()->device);
#else
    ImGui_ImplVulkan_InitInfo info {};

	/*
	struct ImGui_ImplVulkan_InitInfo
	{
    VkInstance          Instance;
    VkPhysicalDevice    PhysicalDevice;
    VkDevice            Device;
    uint32_t            QueueFamily;
    VkQueue             Queue;
    VkPipelineCache     PipelineCache;
    VkDescriptorPool    DescriptorPool;
    uint32_t            MinImageCount;          // >= 2
    uint32_t            ImageCount;             // >= MinImageCount
    VkSampleCountFlagBits        MSAASamples;   // >= VK_SAMPLE_COUNT_1_BIT
    const VkAllocationCallbacks* Allocator;
    void                (*CheckVkResultFn)(VkResult err);
	};
	*/

	// TODO ImGui_ImplVulkan_InitInfo* info, VkRenderPass render_pass are required
#endif

	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

#ifdef _WIN32
		ImGui_ImplDX12_NewFrame();
#elif __APPLE__
		ImGui_ImplMetal_NewFrame();
#else
		ImGui_ImplVulkan_NewFrame();
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Window");

			ImGui::Text("Hello, Altseed");

			ImGui::End();
		}
		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(platform->GetCurrentScreen(color, true));

		// imgui
#ifdef _WIN32
		auto cl = static_cast<LLGI::CommandListDX12*>(commandList);
		cl->GetCommandList()->SetDescriptorHeaps(1, &srvDescHeap);
#elif __APPLE__
		auto cl = static_cast<LLGI::CommandListMetal*>(commandList);
#else
		auto cl = static_cast<LLGI::CommandListVulkan*>(commandList);
#endif
		ImGui::Render();

#ifdef _WIN32
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cl->GetCommandList());
#elif __APPLE__
		ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cl->GetCommandList(), cl->GetImpl()->commandBuffer, cl->GetImpl()->renderEncoder);
#else
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cl->GetCommandBuffer());
#endif

		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();

		// glfwSwapBuffers(window);
		glfwPollEvents();
	}

	graphics->WaitFinish();

#ifdef _WIN32
	ImGui_ImplDX12_Shutdown();
#elif __APPLE__
	ImGui_ImplMetal_Shutdown();
#else
	ImGui_ImplVulkan_Shutdown();
#endif

	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

#ifdef _WIN32
	LLGI::SafeRelease(srvDescHeap);
#elif __APPLE__
#else
	// TODO
#endif

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	delete llgiwindow;

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}
