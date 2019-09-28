#include "test.h"

void test_empty(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("Empty", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(deviceType, window.get());

	auto graphics = platform->CreateGraphics();

	while (count < 1000)
	{
		if (!platform->NewFrame())
			break;

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}