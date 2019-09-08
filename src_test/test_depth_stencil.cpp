#include "TestHelper.h"
#include "test.h"

#include <fstream>
#include <iostream>
#include <map>

void test_depth_stencil(LLGI::DeviceType deviceType, bool is_test_depth, bool is_test_stencil)
{
	auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(LLGI::DeviceType::Default));

	int count = 0;

	auto platform = LLGI::CreateSharedPtr(LLGI::CreatePlatform(LLGI::DeviceType::Default));
	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());
	auto sfMemoryPool = LLGI::CreateSharedPtr(graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128));
	auto commandList = LLGI::CreateSharedPtr(graphics->CreateCommandList(sfMemoryPool.get()));

	std::shared_ptr<LLGI::Shader> shader_vs = nullptr;
	std::shared_ptr<LLGI::Shader> shader_ps = nullptr;

	TestHelper::CreateShader(graphics.get(), deviceType, "simple_rectangle.vert", "simple_rectangle.frag", shader_vs, shader_ps);

	std::shared_ptr<LLGI::VertexBuffer> vb1;
	std::shared_ptr<LLGI::IndexBuffer> ib1;
	TestHelper::CreateRectangle(
		graphics.get(), LLGI::Vec3F(-0.5, 0.5, 0.5), LLGI::Vec3F(0.5, -0.5, 0.5), LLGI::Color8(), LLGI::Color8(), vb1, ib1);

	std::shared_ptr<LLGI::VertexBuffer> vb2;
	std::shared_ptr<LLGI::IndexBuffer> ib2;
	TestHelper::CreateRectangle(
		graphics.get(), LLGI::Vec3F(-0.2, 0.2, 0.8), LLGI::Vec3F(0.7, -0.7, 0.8), LLGI::Color8(), LLGI::Color8(), vb2, ib2);

	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pips;

	while (count < 1000)
	{
		platform->NewFrame();
		sfMemoryPool->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		auto renderPass = graphics->GetCurrentScreen(color, true, true);
		auto renderPassPipelineState = LLGI::CreateSharedPtr(renderPass->CreateRenderPassPipelineState());

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

			if (is_test_depth)
			{
				pip->IsDepthTestEnabled = true;
				pip->IsDepthWriteEnabled = true;	
			}

			if (is_test_stencil)
			{
				pip->IsStencilTestEnabled = true;
			}

			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs.get());
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps.get());
			pip->SetRenderPassPipelineState(renderPassPipelineState.get());
			pip->Compile();

			pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
		}

		commandList->Begin();
		commandList->BeginRenderPass(renderPass);

		commandList->SetVertexBuffer(vb1.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib1.get());
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->Draw(2);

		commandList->SetVertexBuffer(vb2.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib2.get());
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->Draw(2);

		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList.get());

		platform->Present();
		count++;
	}

	pips.clear();

	graphics->WaitFinish();

	LLGI::SafeRelease(shader_vs);
	LLGI::SafeRelease(shader_ps);
}

void test_depth(LLGI::DeviceType deviceType) { test_depth_stencil(deviceType, true, false); }

void test_stencil(LLGI::DeviceType deviceType) { test_depth_stencil(deviceType, false, true); }
