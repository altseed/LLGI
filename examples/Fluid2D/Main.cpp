#include <LLGI.CommandList.h>
#include <LLGI.Graphics.h>
#include <LLGI.Platform.h>
#include <LLGI.Buffer.h>
#include <LLGI.PipelineState.h>
#include "Fluid2D.h"

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")
#endif

int main()
{
    LLGI::PlatformParameter pp;
    pp.Device = LLGI::DeviceType::Default;
    pp.WaitVSync = true;
    auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("Fluid2D", LLGI::Vec2I(1280, 720)));
    auto platform = LLGI::CreatePlatform(pp, window.get());

    auto graphics = platform->CreateGraphics();
    auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);
    auto commandList = graphics->CreateCommandList(sfMemoryPool);

    const auto fluid2d = std::make_unique<Fluid2D>(graphics, pp.Device);
    fluid2d->Initialize(graphics, commandList);
    
    bool initialized = false;

    while (true)
    {
        if (!platform->NewFrame()) break;

        sfMemoryPool->NewFrame();

        fluid2d->Update(graphics, commandList);

        const LLGI::Color8 color = { 50, 50, 50, 255 };
        auto renderPass = platform->GetCurrentScreen(color, true, false);

        commandList->WaitUntilCompleted();

        commandList->Begin();
        fluid2d->Render(graphics, commandList, renderPass);
        commandList->End();
        graphics->Execute(commandList);
        graphics->WaitFinish();

        platform->Present();
    }

    graphics->WaitFinish();

    LLGI::SafeRelease(sfMemoryPool);
    LLGI::SafeRelease(commandList);
    LLGI::SafeRelease(graphics);
    LLGI::SafeRelease(platform);

    return 0;
}