#include "test.h"

void test_clear_update()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	while (count < 1000)
	{
		platform->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen());
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear( color);
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

void test_clear()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
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

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen());
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(color);
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