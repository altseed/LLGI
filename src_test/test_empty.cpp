#include "test.h"

void test_empty(LLGI::DeviceType deviceType)
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(deviceType);
	auto graphics = platform->CreateGraphics();

	while (count < 1000)
	{
		platform->NewFrame();
		platform->Present();
		count++;
	}

	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}