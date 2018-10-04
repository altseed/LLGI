#include "test.h"

void test_empty()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
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