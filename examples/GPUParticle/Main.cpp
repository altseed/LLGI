
#include <iostream>
#include <unordered_map>
#include <LLGI.CommandList.h>
#include <LLGI.Graphics.h>
#include <LLGI.Platform.h>
#include <LLGI.Shader.h>
#include <LLGI.Compiler.h>
#include <LLGI.VertexBuffer.h>
#include <LLGI.IndexBuffer.h>
#include <LLGI.PipelineState.h>
#include "GPUParticle.h"

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")
#endif



class GPUParticleContext;


namespace LLGI {
	void SetIsGPUDebugEnabled(bool value);
}


int main()
{
	int count = 0;

	LLGI::SetIsGPUDebugEnabled(true);

	LLGI::PlatformParameter pp;
	pp.Device = LLGI::DeviceType::Default;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("ClearUpdate", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(pp, window.get());

	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);

	std::array<LLGI::CommandList*, 3> commandLists;
	for (size_t i = 0; i < commandLists.size(); i++)
		commandLists[i] = graphics->CreateCommandList(sfMemoryPool);


	auto particleContext = std::make_unique<GPUParticleContext>(graphics, pp.Device, platform->GetMaxFrameCount(), 512);


	//GPUParticleRenderPass gpuParticleRenderPass(graphics, pp.Device, platform->GetMaxFrameCount());


	particleContext->Emit(10, LLGI::Vec3F(0, 0, 0), LLGI::Vec3F(0.0001, 0, 0));
	particleContext->Emit(10, LLGI::Vec3F(0.1, 0.1, 0), LLGI::Vec3F(0, 0.0001, 0));
	particleContext->Emit(10, LLGI::Vec3F(0.2, 0.2, 0), LLGI::Vec3F(-0.0001, 0, 0));
	particleContext->Emit(10, LLGI::Vec3F(0.3, 0.3, 0), LLGI::Vec3F(0, -0.0001, 0));




	while (true)
	{




		if (!platform->NewFrame())
			break;

		sfMemoryPool->NewFrame();

		particleContext->NewFrame();

		LLGI::Color8 color;
		color.R = (count + 200) % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;


		auto renderPass = platform->GetCurrentScreen(color, true, false); // TODO: isDepthClear is false, because it fails with dx12.

		auto commandList = commandLists[count % commandLists.size()];
		commandList->WaitUntilCompleted();





		commandList->Begin();


		particleContext->Render(renderPass, commandList);

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	for (size_t i = 0; i < commandLists.size(); i++)
		LLGI::SafeRelease(commandLists[i]);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	return 0;
}
