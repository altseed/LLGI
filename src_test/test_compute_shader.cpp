#include "TestHelper.h"
#include "test.h"

#include <LLGI.ComputeBuffer.h>
#include <Utils/LLGI.CommandListPool.h>

struct ComputeData
{
	float value;
};

void test_compute_shader(LLGI::DeviceType deviceType)
{
	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("Capture", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreateSharedPtr(LLGI::CreatePlatform(pp, window.get()));
	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());

	auto sfMemoryPool = LLGI::CreateSharedPtr(graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128));

	auto commandListPool = std::make_shared<LLGI::CommandListPool>(graphics.get(), sfMemoryPool.get(), 3);

	std::shared_ptr<LLGI::Shader> shader_cs = nullptr;

	TestHelper::CreateComputeShader(graphics.get(), deviceType, "basic.comp", shader_cs);

	auto pip = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
	pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32_FLOAT;
	pip->VertexLayoutNames[0] = "value";
	pip->VertexLayoutCount = 1;
	pip->SetShader(LLGI::ShaderStageType::Compute, shader_cs.get());
	pip->Compile();

	int dataSize = 256;

	std::shared_ptr<LLGI::ComputeBuffer> computeBuffer;
	computeBuffer = LLGI::CreateSharedPtr(graphics->CreateComputeBuffer(sizeof(ComputeData) * dataSize));

	{
		auto data = (ComputeData*)computeBuffer->Lock();
		for (int i = 0; i < dataSize; i++)
		{
			data[i].value = (float)i;
		}
		computeBuffer->Unlock();
	}

	std::shared_ptr<LLGI::ConstantBuffer> constantBuffer;
	constantBuffer = LLGI::CreateSharedPtr(graphics->CreateConstantBuffer(sizeof(float)));

	{
		auto data = (float*)constantBuffer->Lock();
		data[0] = 100;
		constantBuffer->Unlock();
	}

	if (!platform->NewFrame())
		return;

	sfMemoryPool->NewFrame();

	auto commandList = commandListPool->Get();
	commandList->Begin();
	commandList->UpdateDataToGPU(computeBuffer.get());
	commandList->UpdateData(constantBuffer.get());
	commandList->BeginComputePass();
	commandList->SetPipelineState(pip.get());
	commandList->SetComputeBuffer(computeBuffer.get());
	commandList->SetConstantBuffer(constantBuffer.get(), LLGI::ShaderStageType::Compute);
	commandList->Dispatch(dataSize, 1, 1);
	commandList->EndComputePass();
	commandList->UpdateDataToCPU(computeBuffer.get());
	commandList->End();

	graphics->Execute(commandList);
	graphics->WaitFinish();

	{
		auto data = (ComputeData*)computeBuffer->Read();
		for (int i = 0; i < dataSize; i++)
		{
			std::cout << "data[" << i << "] = " << data[i].value << std::endl;
		}
	}

	platform->Present();
}

TestRegister ComputeShader_Basic("ComputeShader.Basic", [](LLGI::DeviceType device) -> void { test_compute_shader(device); });
