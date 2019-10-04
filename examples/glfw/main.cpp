
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#endif

#include <GLFW/glfw3native.h>
#include <LLGI.CommandList.h>
#include <LLGI.Graphics.h>
#include <LLGI.Platform.h>

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")
#endif

class LLGIWindow : public LLGI::Window
{
	GLFWwindow* window_ = nullptr;

public:
	LLGIWindow(GLFWwindow* window) : window_(window) {}

	bool OnNewFrame() { return glfwWindowShouldClose(window_) == GL_FALSE; }

	void* GetNativePtr(int32_t index)
	{

#ifdef _WIN32
		if (index == 0)
		{
			return glfwGetWin32Window(window_);
		}

		return (HINSTANCE)GetModuleHandle(0);
#endif
	}

	LLGI::Vec2I GetWindowSize() const
	{
		int w, h;
		glfwGetWindowSize(window_, &w, &h);
		return LLGI::Vec2I(w, h);
	}
};

int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto window = glfwCreateWindow(1280, 720, "Example glfw", nullptr, nullptr);

	auto llgiwindow = new LLGIWindow(window);

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default, llgiwindow);
	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);
	auto commandList = graphics->CreateCommandList(sfMemoryPool);

	LLGI::Color8 color;
	color.R = 255;
	color.G = 0;
	color.B = 0;
	color.A = 255;

	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(platform->GetCurrentScreen(color, true));
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();

		// glfwSwapBuffers(window);
		glfwPollEvents();
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	delete llgiwindow;

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}