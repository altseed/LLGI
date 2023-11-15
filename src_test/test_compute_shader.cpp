#include "TestHelper.h"
#include "test.h"

#include <LLGI.Buffer.h>
#include <Utils/LLGI.CommandListPool.h>

struct InputData
{
	float value1;
	float value2;
};

struct OutputData
{
	float value;
};

void test_compute_shader(LLGI::DeviceType deviceType, bool is_read_only)
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

	if (is_read_only)
	{
		TestHelper::CreateComputeShader(graphics.get(), deviceType, "readwrite.comp", shader_cs);
	}
	else
	{
		TestHelper::CreateComputeShader(graphics.get(), deviceType, "basic.comp", shader_cs);
	}

	auto pip = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
	pip->SetShader(LLGI::ShaderStageType::Compute, shader_cs.get());
	if (!pip->Compile())
	{
		abort();
	}

	int dataSize = 256;

	std::vector<InputData> inputData;
	inputData.resize(dataSize);
	for (int i = 0; i < dataSize; i++)
	{
		inputData[i].value1 = (float)i * 2;
		inputData[i].value2 = (float)i * 2 + 1;
	}

	std::shared_ptr<LLGI::Buffer> inputBuffer;
	inputBuffer = LLGI::CreateSharedPtr(
		graphics->CreateBuffer(LLGI::BufferUsageType::MapWrite | LLGI::BufferUsageType::CopySrc, sizeof(InputData) * dataSize));

	{
		auto data = (InputData*)inputBuffer->Lock();
		for (int i = 0; i < dataSize; i++)
		{
			data[i] = inputData[i];
		}
		inputBuffer->Unlock();
	}

	std::shared_ptr<LLGI::Buffer> outputBuffer;
	outputBuffer = LLGI::CreateSharedPtr(
		graphics->CreateBuffer(LLGI::BufferUsageType::MapRead | LLGI::BufferUsageType::CopyDst, sizeof(OutputData) * dataSize));

	std::shared_ptr<LLGI::Buffer> inputComputeBuffer;
	inputComputeBuffer = LLGI::CreateSharedPtr(
		graphics->CreateBuffer(LLGI::BufferUsageType::Compute | LLGI::BufferUsageType::CopyDst, sizeof(InputData) * dataSize));

	std::shared_ptr<LLGI::Buffer> outputComputeBuffer;
	outputComputeBuffer = LLGI::CreateSharedPtr(
		graphics->CreateBuffer(LLGI::BufferUsageType::Compute | LLGI::BufferUsageType::CopySrc, sizeof(OutputData) * dataSize));

	std::shared_ptr<LLGI::Buffer> constantBuffer;
	constantBuffer =
		LLGI::CreateSharedPtr(graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(float)));

	const int offsetValue = 100;

	{
		auto data = (float*)constantBuffer->Lock();
		data[0] = offsetValue;
		constantBuffer->Unlock();
	}

	if (!platform->NewFrame())
		return;

	sfMemoryPool->NewFrame();

	auto commandList = commandListPool->Get();
	commandList->Begin();
	commandList->CopyBuffer(inputBuffer.get(), inputComputeBuffer.get());
	commandList->BeginComputePass();
	commandList->SetPipelineState(pip.get());
	commandList->SetComputeBuffer(inputComputeBuffer.get(), sizeof(InputData), 0, is_read_only);
	commandList->SetComputeBuffer(outputComputeBuffer.get(), sizeof(OutputData), 1, false);
	commandList->SetConstantBuffer(constantBuffer.get(), 0);
	commandList->Dispatch(dataSize, 1, 1, 1, 1, 1);
	commandList->EndComputePass();
	commandList->CopyBuffer(outputComputeBuffer.get(), outputBuffer.get());
	commandList->End();

	graphics->Execute(commandList);
	graphics->WaitFinish();

	{
		auto dst = (OutputData*)outputBuffer->Lock();
		if (dst == nullptr)
		{
			abort();
		}

		for (int i = 0; i < dataSize; i++)
		{
			const auto expected = inputData[i].value1 * inputData[i].value2 + offsetValue;
			const auto actual = dst[i].value;
			if (expected != actual)
			{
				abort();
			}
		}

		outputBuffer->Unlock();
	}

	platform->Present();
}

void test_compute_shader_texture(LLGI::DeviceType deviceType)
{
	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("ComputeShaderTexture", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreateSharedPtr(LLGI::CreatePlatform(pp, window.get()));
	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());

	auto sfMemoryPool = LLGI::CreateSharedPtr(graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128));

	auto commandListPool = std::make_shared<LLGI::CommandListPool>(graphics.get(), sfMemoryPool.get(), 3);

	std::shared_ptr<LLGI::Shader> shader_cs = nullptr;

	TestHelper::CreateComputeShader(graphics.get(), deviceType, "readwrite_texture.comp", shader_cs);

	auto pip = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
	pip->SetShader(LLGI::ShaderStageType::Compute, shader_cs.get());
	if (!pip->Compile())
	{
		abort();
	}

	LLGI::TextureParameter texParamRead1;
	texParamRead1.Size = {1, 1, 1};
	// For Vulkan
	texParamRead1.Format = LLGI::TextureFormatType::R32G32B32A32_FLOAT;
	// For DX12
	texParamRead1.Usage = LLGI::TextureUsageType::Storage;
	auto texRead1 = LLGI::CreateSharedPtr(graphics->CreateTexture(texParamRead1));

	if (auto data = texRead1->Lock())
	{
		auto p = static_cast<float*>(data);
		p[0] = 0.25f;
		p[1] = 0.25f;
		p[2] = 0;
		p[3] = 0.25f;
		texRead1->Unlock();
	}

	LLGI::TextureParameter texParamRead2;
	texParamRead2.Size = {1, 1, 1};
	// For Vulkan
	texParamRead2.Format = LLGI::TextureFormatType::R32G32B32A32_FLOAT;
	// For DX12
	texParamRead2.Usage = LLGI::TextureUsageType::Storage;
	auto texRead2 = LLGI::CreateSharedPtr(graphics->CreateTexture(texParamRead2));

	if (auto data = texRead2->Lock())
	{
		auto p = static_cast<float*>(data);
		p[0] = 0.25f;
		p[1] = 0;
		p[2] = 0.25f;
		p[3] = 0.25f;
		texRead2->Unlock();
	}

	LLGI::TextureParameter texParamWrite;
	texParamWrite.Size = {1, 1, 1};
	texParamWrite.Usage = LLGI::TextureUsageType::Storage;
	auto texWrite = LLGI::CreateSharedPtr(graphics->CreateTexture(texParamWrite));

	if (!platform->NewFrame())
		return;

	sfMemoryPool->NewFrame();

	auto commandList = commandListPool->Get();
	commandList->Begin();
	commandList->BeginComputePass();
	commandList->SetPipelineState(pip.get());
	commandList->SetTexture(texWrite.get(), LLGI::TextureWrapMode::Clamp, LLGI::TextureMinMagFilter::Nearest, 0);
	commandList->SetTexture(texRead1.get(), LLGI::TextureWrapMode::Clamp, LLGI::TextureMinMagFilter::Nearest, 1);
	commandList->SetTexture(texRead2.get(), LLGI::TextureWrapMode::Clamp, LLGI::TextureMinMagFilter::Nearest, 2);
	commandList->Dispatch(1, 1, 1, 1, 1, 1);
	commandList->EndComputePass();
	commandList->End();

	graphics->Execute(commandList);
	graphics->WaitFinish();

	std::vector<uint8_t> result;
	if (texWrite->GetData(result))
	{
		if (!(result[0] == 128 && result[1] == 64 && result[2] == 64 && result[3] == 128))
		{
			abort();
		}
	}

	platform->Present();
}

TestRegister ComputeShader_Basic("ComputeShader.Basic", [](LLGI::DeviceType device) -> void { test_compute_shader(device, false); });

TestRegister ComputeShader_Basic_ReadOnly("ComputeShader.Basic_ReadOnly",
										  [](LLGI::DeviceType device) -> void { test_compute_shader(device, true); });

TestRegister ComputeShader_Basic_Texture("ComputeShader.Basic_Texture",
										 [](LLGI::DeviceType device) -> void { test_compute_shader_texture(device); });
