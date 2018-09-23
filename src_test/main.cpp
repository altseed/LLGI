
#include <G3/LLGI.G3.Platform.h>
#include <G3/LLGI.G3.Graphics.h>
#include <G3/LLGI.G3.CommandList.h>

void test_clear()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::PlatformType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	commandList->Begin();
	commandList->Clear();
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

int main()
{
	test_clear();


	return 0;
}
