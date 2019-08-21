#include "test.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../thirdparty/stb/stb_image_write.h"

#define CAPTURE_TEST 0

void test_clear_update(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto platform = LLGI::CreatePlatform(deviceType);
	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);
	auto commandList = graphics->CreateCommandList(sfMemoryPool);

	while (count < 1000)
	{
		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

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

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

void test_clear(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto platform = LLGI::CreatePlatform(deviceType);
	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);
	auto commandList = graphics->CreateCommandList(sfMemoryPool);

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

		auto renderPass = graphics->GetCurrentScreen(color, true);

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

#if CAPTURE_TEST
		{
			auto texture = renderPass->GetColorBuffer(0);
			auto data = graphics->CaptureRenderTarget(texture);
			stbi_write_png("stbpng.png", texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, 4, data.data(), texture->GetSizeAs2D().X * 4);
		}
#endif
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}
