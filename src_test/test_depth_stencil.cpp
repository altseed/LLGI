#include "TestHelper.h"
#include "test.h"

#include <fstream>
#include <iostream>
#include <map>

enum class DepthStencilTestMode
{
	Depth,
	Stentil,
};

void test_depth_stencil(LLGI::DeviceType deviceType, DepthStencilTestMode mode)
{
	auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(LLGI::DeviceType::Default));

	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("DepthStencil", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(pp, window.get());

	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());
	auto sfMemoryPool = LLGI::CreateSharedPtr(graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128));
	auto commandList = LLGI::CreateSharedPtr(graphics->CreateCommandList(sfMemoryPool.get()));

	std::shared_ptr<LLGI::Shader> shader_vs = nullptr;
	std::shared_ptr<LLGI::Shader> shader_ps = nullptr;

	TestHelper::CreateShader(graphics.get(), deviceType, "simple_rectangle.vert", "simple_rectangle.frag", shader_vs, shader_ps);

	// Green: near
	std::shared_ptr<LLGI::VertexBuffer> vb1;
	std::shared_ptr<LLGI::IndexBuffer> ib1;
	TestHelper::CreateRectangle(graphics.get(),
								LLGI::Vec3F(-0.5f, 0.5f, 0.5f),
								LLGI::Vec3F(0.5f, -0.5f, 0.5f),
								LLGI::Color8(0, 255, 0, 255),
								LLGI::Color8(),
								vb1,
								ib1);

	// Blue: far
	std::shared_ptr<LLGI::VertexBuffer> vb2;
	std::shared_ptr<LLGI::IndexBuffer> ib2;
	TestHelper::CreateRectangle(graphics.get(),
								LLGI::Vec3F(-0.2f, 0.2f, 0.8f),
								LLGI::Vec3F(0.7f, -0.7f, 0.8f),
								LLGI::Color8(0, 0, 255, 255),
								LLGI::Color8(),
								vb2,
								ib2);

	struct PipelineStateSet
	{
		std::shared_ptr<LLGI::PipelineState> writeState; // write depth or stencil
		std::shared_ptr<LLGI::PipelineState> testState;	 // depth-test or stencil-test
	};
	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, PipelineStateSet> pips;

	// auto screenRenderPass = graphics->GetCurrentScreen(color, true, true);
	// auto depthBuffer = graphics->CreateTexture(LLGI::Vec2I(256, 256), false, true);

	// <SwapChainRenderPass, BackbufferDepthBuffer>
	std::array<LLGI::Texture*, 3> depthBuffers = {};
	std::array<LLGI::RenderPass*, 3> renderPasses = {};

	while (count < 60)
	{
		if (!platform->NewFrame())
		{
			break;
		}

		sfMemoryPool->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		// TODO : fixed swapchin is not good
		int swapIndex = count % 3;
		auto screenRenderPass = platform->GetCurrentScreen(color, true, true);
		LLGI::RenderPass* renderPass;
		if (!renderPasses[swapIndex])
		{
			auto colorBuffer = screenRenderPass->GetRenderTexture(0);
			LLGI::DepthTextureInitializationParameter depthParam;
			depthParam.Size = colorBuffer->GetSizeAs2D();
			auto depthBuffer = graphics->CreateDepthTexture(depthParam);
			assert(depthBuffer != nullptr && depthBuffer->GetType() == LLGI::TextureType::Depth);

			renderPass = graphics->CreateRenderPass((const LLGI::Texture**)&colorBuffer, 1, depthBuffer);
			depthBuffers[swapIndex] = depthBuffer;
			renderPasses[swapIndex] = renderPass;
		}
		else
		{
			renderPass = renderPasses[swapIndex];
		}
		renderPass->SetIsColorCleared(true);
		renderPass->SetClearColor(color);
		renderPass->SetIsDepthCleared(true);

		auto renderPassPipelineState = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPass));

		if (pips.count(renderPassPipelineState) == 0)
		{
			auto writepip = graphics->CreatePiplineState();
			writepip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
			writepip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
			writepip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
			writepip->VertexLayoutNames[0] = "POSITION";
			writepip->VertexLayoutNames[1] = "UV";
			writepip->VertexLayoutNames[2] = "COLOR";
			writepip->VertexLayoutCount = 3;

			writepip->Culling = LLGI::CullingMode::DoubleSide; // TEMP :vulkan

			writepip->IsBlendEnabled = false;

			if (mode == DepthStencilTestMode::Depth)
			{
				writepip->IsDepthWriteEnabled = true;
			}

			writepip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs.get());
			writepip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps.get());
			writepip->SetRenderPassPipelineState(renderPassPipelineState.get());
			writepip->Compile();

			auto testpip = graphics->CreatePiplineState();
			testpip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
			testpip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
			testpip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
			testpip->VertexLayoutNames[0] = "POSITION";
			testpip->VertexLayoutNames[1] = "UV";
			testpip->VertexLayoutNames[2] = "COLOR";
			testpip->VertexLayoutCount = 3;

			testpip->Culling = LLGI::CullingMode::DoubleSide; // TEMP :vulkan

			testpip->IsBlendEnabled = false;

			if (mode == DepthStencilTestMode::Depth)
			{
				testpip->IsDepthTestEnabled = true;
			}

			if (mode == DepthStencilTestMode::Stentil)
			{
				testpip->IsStencilTestEnabled = true;
			}

			testpip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs.get());
			testpip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps.get());
			testpip->SetRenderPassPipelineState(renderPassPipelineState.get());
			testpip->Compile();

			pips[renderPassPipelineState].writeState = LLGI::CreateSharedPtr(writepip);
			pips[renderPassPipelineState].testState = LLGI::CreateSharedPtr(testpip);

			if (TestHelper::GetIsCaptureRequired() && count == 30)
			{
				commandList->WaitUntilCompleted();
				auto texture = platform->GetCurrentScreen(color, true)->GetRenderTexture(0);
				auto data = graphics->CaptureRenderTarget(texture);

				// save
				if (mode == DepthStencilTestMode::Depth)
				{
					Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save("DepthStentil.Depth.png");
				}
				else if (mode == DepthStencilTestMode::Stentil)
				{
					Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, true).Save("DepthStentil.Stentil.png");
				}
			}
		}

		commandList->Begin();
		commandList->BeginRenderPass(renderPass);

		// First, green rectangle.
		commandList->SetVertexBuffer(vb1.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib1.get());
		commandList->SetPipelineState(pips[renderPassPipelineState].writeState.get());
		commandList->Draw(2);

		// Next, blue rectangle.
		commandList->SetVertexBuffer(vb2.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib2.get());
		commandList->SetPipelineState(pips[renderPassPipelineState].testState.get());
		commandList->Draw(2);

		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList.get());

		platform->Present();
		count++;
	}

	for (auto& ptr : depthBuffers)
		LLGI::SafeRelease(ptr);
	for (auto& ptr : renderPasses)
		LLGI::SafeRelease(ptr);
	pips.clear();

	graphics->WaitFinish();
}

TestRegister DepthStentil_Depth("DepthStentil.Depth",
								[](LLGI::DeviceType device) -> void { test_depth_stencil(device, DepthStencilTestMode::Depth); });

TestRegister DepthStentil_Stencil("DepthStentil.Stencil",
								  [](LLGI::DeviceType device) -> void { test_depth_stencil(device, DepthStencilTestMode::Stentil); });
