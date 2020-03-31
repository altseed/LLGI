#include "TestHelper.h"
#include "test.h"
#include <array>

void test_clear_update(LLGI::DeviceType deviceType)
{
	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("ClearUpdate", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(pp, window.get());

	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);

	std::array<LLGI::CommandList*, 3> commandLists;
	for (int i = 0; i < commandLists.size(); i++)
		commandLists[i] = graphics->CreateCommandList(sfMemoryPool);

	while (count < 1000)
	{
		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

		LLGI::Color8 color;
		color.R = (count + 200) % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		auto commandList = commandLists[count % commandLists.size()];
		commandList->WaitUntilCompleted();

		commandList->Begin();
		commandList->BeginRenderPass(
			platform->GetCurrentScreen(color, true, false)); // TODO: isDepthClear is false, because it fails with dx12.
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (TestHelper::GetIsCaptureRequired() && count == 5)
		{
			commandList->WaitUntilCompleted();
			auto texture = platform->GetCurrentScreen(color, true)->GetRenderTexture(0);
			auto data = graphics->CaptureRenderTarget(texture);

			// save
			Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save("ClearUpdate.png");
			break;
		}
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	for (int i = 0; i < commandLists.size(); i++)
		LLGI::SafeRelease(commandLists[i]);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

void test_clear(LLGI::DeviceType deviceType)
{
	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("Clear", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(pp, window.get());

	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);

	std::array<LLGI::CommandList*, 3> commandLists;
	for (int i = 0; i < commandLists.size(); i++)
		commandLists[i] = graphics->CreateCommandList(sfMemoryPool);

	LLGI::Color8 color;
	color.R = 255;
	color.G = 0;
	color.B = 0;
	color.A = 255;

	while (count < 1000)
	{
		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		auto commandList = commandLists[count % commandLists.size()];
		commandList->WaitUntilCompleted();

		commandList->Begin();
		commandList->BeginRenderPass(
			platform->GetCurrentScreen(color, true, false)); // TODO: isDepthClear is false, because it fails with dx12.
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (TestHelper::GetIsCaptureRequired() && count == 5)
		{
			commandList->WaitUntilCompleted();
			auto texture = platform->GetCurrentScreen(color, true)->GetRenderTexture(0);
			auto data = graphics->CaptureRenderTarget(texture);

			// save
			Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save("Clear.png");
			break;
		}
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	for (int i = 0; i < commandLists.size(); i++)
		LLGI::SafeRelease(commandLists[i]);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)

TEST(Clear, Basic) { test_clear(LLGI::DeviceType::Default); }

TEST(Clear, Update) { test_clear_update(LLGI::DeviceType::Default); }

#endif
