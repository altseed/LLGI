#include "TestHelper.h"
#include "test.h"

// #define CAPTURE_TEST 1

#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)
class ClearTest : public ::testing::Test
{
};

TEST_F(ClearTest, Clear1) {}
#endif

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
		commandList->BeginRenderPass(platform->GetCurrentScreen(color, true));
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

		// It need to create a command buffer between NewFrame and Present.
		// Because get current screen returns other values by every frame.
		commandList->Begin();
		commandList->BeginRenderPass(platform->GetCurrentScreen(color, true));
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

#if CAPTURE_TEST
		if (count == 5)
		{
			// TODO wait to finish commandList
			auto texture = platform->GetCurrentScreen(color, true)->GetColorBuffer(0);
			auto data = graphics->CaptureRenderTarget(texture);

			// save
			Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save("stbpng.png");

			// test
			int rate = Bitmap2D::CompareBitmap(
				Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true), Bitmap2D("stbpng.png"), 5);
			std::cout << rate;
		}
#endif
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}
