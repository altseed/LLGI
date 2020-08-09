
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


class GPUParticleRenderPass
{
public:
	struct SimpleVertex
	{
		LLGI::Vec3F Pos;
		LLGI::Vec2F UV;
		LLGI::Color8 Color;
	};

	GPUParticleRenderPass(LLGI::Graphics* graphics, LLGI::DeviceType deviceType, int frameCount)
		: graphcis_(graphics)
		, shader_(std::make_unique<Shader>(graphics, deviceType,
			"C:/Proj/LN/Lumino/build/ExternalSource/Effekseer/Dev/Cpp/3rdParty/LLGI/examples/GPUParticle/Shaders/HLSL_DX12/perticle-update.vert",
			"C:/Proj/LN/Lumino/build/ExternalSource/Effekseer/Dev/Cpp/3rdParty/LLGI/examples/GPUParticle/Shaders/HLSL_DX12/perticle-update.frag"))
	{
		const LLGI::Vec3F ul = LLGI::Vec3F(-0.5, 0.5, 0.5);
		const LLGI::Vec3F lr = LLGI::Vec3F(0.5, -0.5, 0.5);

		vb_ = LLGI::CreateSharedPtr(graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4));
		ib_ = LLGI::CreateSharedPtr(graphics->CreateIndexBuffer(2, 6));
		auto vb_buf = (SimpleVertex*)vb_->Lock();
		vb_buf[0].Pos = LLGI::Vec3F(ul.X, ul.Y, ul.Z);
		vb_buf[1].Pos = LLGI::Vec3F(lr.X, ul.Y, ul.Z);
		vb_buf[2].Pos = LLGI::Vec3F(lr.X, lr.Y, lr.Z);
		vb_buf[3].Pos = LLGI::Vec3F(ul.X, lr.Y, lr.Z);

		vb_buf[0].UV = LLGI::Vec2F(0.0f, 0.0f);
		vb_buf[1].UV = LLGI::Vec2F(1.0f, 0.0f);
		vb_buf[2].UV = LLGI::Vec2F(1.0f, 1.0f);
		vb_buf[3].UV = LLGI::Vec2F(0.0f, 1.0f);

		vb_buf[0].Color = LLGI::Color8(0, 0, 0, 1);
		vb_buf[1].Color = LLGI::Color8(0, 0, 0, 1);
		vb_buf[2].Color = LLGI::Color8(0, 0, 0, 1);
		vb_buf[3].Color = LLGI::Color8(0, 0, 0, 1);

		vb_->Unlock();

		auto ib_buf = (uint16_t*)ib_->Lock();
		ib_buf[0] = 0;
		ib_buf[1] = 1;
		ib_buf[2] = 2;
		ib_buf[3] = 0;
		ib_buf[4] = 2;
		ib_buf[5] = 3;
		ib_->Unlock();
	}

	void draw(LLGI::RenderPass* renderPass, LLGI::CommandList* commandList)
	{
		auto renderPassState = LLGI::CreateSharedPtr(graphcis_->CreateRenderPassPipelineState(renderPass));

		std::shared_ptr<LLGI::PipelineState> pipeline;
		auto itr = pipelineCache_.find(renderPassState);
		if (itr == pipelineCache_.end()) {
			auto pip = graphcis_->CreatePiplineState();
			pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
			pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
			pip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
			pip->VertexLayoutNames[0] = "POSITION";
			pip->VertexLayoutNames[1] = "UV";
			pip->VertexLayoutNames[2] = "COLOR";
			pip->VertexLayoutCount = 3;
			pip->Topology = LLGI::TopologyType::Triangle;

			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_->vertexShader());
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_->pixelShader());
			pip->SetRenderPassPipelineState(renderPassState.get());
			pip->Compile();
			pipeline = LLGI::CreateSharedPtr(pip);
			pipelineCache_[renderPassState] = pipeline;
		}
		else {
			pipeline = itr->second;
		}


		commandList->SetVertexBuffer(vb_.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib_.get());
		commandList->SetPipelineState(pipeline.get());
		commandList->Draw(2);
	}

private:
	LLGI::Graphics* graphcis_;
	std::unique_ptr<Shader> shader_;
	std::shared_ptr<LLGI::VertexBuffer> vb_;
	std::shared_ptr<LLGI::IndexBuffer> ib_;
	std::unordered_map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pipelineCache_;

};

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


	particleContext->Emit(10, LLGI::Vec3F(0, 0, 0), LLGI::Vec3F(0.1, 0, 0));
	particleContext->Emit(10, LLGI::Vec3F(0, 0, 0), LLGI::Vec3F(0, 0.1, 0));
	particleContext->Emit(10, LLGI::Vec3F(0, 0, 0), LLGI::Vec3F(-0.1, 0, 0));
	particleContext->Emit(10, LLGI::Vec3F(0, 0, 0), LLGI::Vec3F(0, -0.1, 0));


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

		commandList->BeginRenderPass(renderPass);

		//gpuParticleRenderPass.draw();

		commandList->EndRenderPass();
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
