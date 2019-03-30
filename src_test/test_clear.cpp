#include "test.h"

void test_clear_update(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto platform = LLGI::CreatePlatform(deviceType);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	while (count < 1000)
	{
		platform->NewFrame();
		graphics->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen(color, true));
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

void test_clear(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto platform = LLGI::CreatePlatform(deviceType);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	LLGI::Color8 color;
	color.R = 255;
	color.G = 0;
	color.B = 0;
	color.A = 255;

	while (count < 1000)
	{
		platform->NewFrame();
		graphics->NewFrame();

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen(color, true));
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}
