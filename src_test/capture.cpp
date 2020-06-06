#include "TestHelper.h"
#include "test.h"
#include <Utils/LLGI.CommandListPool.h>
#include <array>

void test_capture(LLGI::DeviceType deviceType, LLGI::Vec2I windowSize)
{
	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("Capture", windowSize));
	auto platform = LLGI::CreateSharedPtr(LLGI::CreatePlatform(pp, window.get()));
	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());

	auto sfMemoryPool = LLGI::CreateSharedPtr(graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128));

	auto commandListPool = std::make_shared<LLGI::CommandListPool>(graphics.get(), sfMemoryPool.get(), 3);

	std::shared_ptr<LLGI::Shader> shader_vs = nullptr;
	std::shared_ptr<LLGI::Shader> shader_ps = nullptr;

	TestHelper::CreateShader(graphics.get(), deviceType, "simple_rectangle.vert", "simple_rectangle.frag", shader_vs, shader_ps);

	std::shared_ptr<LLGI::VertexBuffer> vb;
	std::shared_ptr<LLGI::IndexBuffer> ib;
	TestHelper::CreateRectangle(graphics.get(),
								LLGI::Vec3F(-0.5, 0.5, 0.5),
								LLGI::Vec3F(0.5, -0.5, 0.5),
								LLGI::Color8(255, 255, 255, 255),
								LLGI::Color8(0, 255, 0, 255),
								vb,
								ib);

	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pips;

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

		auto renderPass = platform->GetCurrentScreen(color, true, false); // TODO: isDepthClear is false, because it fails with dx12.
		auto renderPassPipelineState = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPass));

		if (pips.count(renderPassPipelineState) == 0)
		{
			auto pip = graphics->CreatePiplineState();
			pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
			pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
			pip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
			pip->VertexLayoutNames[0] = "POSITION";
			pip->VertexLayoutNames[1] = "UV";
			pip->VertexLayoutNames[2] = "COLOR";
			pip->VertexLayoutCount = 3;

			pip->Culling = LLGI::CullingMode::DoubleSide; // TEMP :vulkan
			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs.get());
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps.get());
			pip->SetRenderPassPipelineState(renderPassPipelineState.get());
			pip->Compile();

			pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
		}

		auto commandList = commandListPool->Get();
		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->Draw(2);
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (TestHelper::GetIsCaptureRequired() && count == 5)
		{
			commandList->WaitUntilCompleted();
			auto texture = platform->GetCurrentScreen(LLGI::Color8(), true)->GetRenderTexture(0);
			auto data = graphics->CaptureRenderTarget(texture);

			// save
			std::string path = "Capture_" + std::to_string(windowSize.X) + "_" + std::to_string(windowSize.Y) + ".png";
			Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save(path.c_str());
			break;
		}
	}

	pips.clear();
}

#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)

TEST(Capture, Size1279) { test_capture(LLGI::DeviceType::Default, LLGI::Vec2I(1279, 719)); }

TEST(Capture, Size800) { test_capture(LLGI::DeviceType::Default, LLGI::Vec2I(800, 600)); }

TEST(Capture, Size1280) { test_capture(LLGI::DeviceType::Default, LLGI::Vec2I(1280, 720)); }

#endif
