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
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(graphics->GetCurrentScreen(), color);
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

	commandList->Begin();
	commandList->SetScissor(0, 0, 1280, 720);
	commandList->Clear(graphics->GetCurrentScreen(), color);
	commandList->End();

	while (count < 1000)
	{
		platform->NewFrame();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}