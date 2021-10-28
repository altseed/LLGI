#include "TestHelper.h"
#include "test.h"

#include <LLGI.Buffer.h>
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

	std::shared_ptr<LLGI::Buffer> read;
	read = LLGI::CreateSharedPtr(graphics->CreateBuffer(LLGI::BufferUsageType::Compute, sizeof(ComputeData) * dataSize));

	{
		auto data = (ComputeData*)read->Lock();
		for (int i = 0; i < dataSize; i++)
		{
			data[i].value = (float)i;
		}
		read->Unlock();
	}

	std::shared_ptr<LLGI::Buffer> write;
	write = LLGI::CreateSharedPtr(graphics->CreateBuffer(LLGI::BufferUsageType::Compute, sizeof(ComputeData) * dataSize));

	std::shared_ptr<LLGI::Buffer> constantBuffer;
	constantBuffer = LLGI::CreateSharedPtr(graphics->CreateBuffer(LLGI::BufferUsageType::Constant, sizeof(float)));

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
	commandList->UploadBuffer(read.get());
	commandList->UploadBuffer(constantBuffer.get());
	commandList->BeginComputePass();
	commandList->SetPipelineState(pip.get());
	commandList->SetComputeBuffer(read.get(), 0);
	commandList->SetComputeBuffer(write.get(), 1);
	commandList->SetConstantBuffer(constantBuffer.get(), LLGI::ShaderStageType::Compute);
	commandList->Dispatch(dataSize, 1, 1);
	commandList->EndComputePass();
	commandList->ReadBackBuffer(read.get());
	commandList->ReadBackBuffer(write.get());
	commandList->End();

	graphics->Execute(commandList);
	graphics->WaitFinish();

	{
		auto data = (ComputeData*)read->Read();
		for (int i = 0; i < dataSize; i++)
		{
			std::cout << "read[" << i << "] = " << data[i].value << std::endl;
		}
	}
	{
		auto data = (ComputeData*)write->Read();
		for (int i = 0; i < dataSize; i++)
		{
			std::cout << "write[" << i << "] = " << data[i].value << std::endl;
		}
	}

	platform->Present();
}

TestRegister ComputeShader_Basic("ComputeShader.Basic", [](LLGI::DeviceType device) -> void { test_compute_shader(device); });
