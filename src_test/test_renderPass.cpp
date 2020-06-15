#include "TestHelper.h"
#include "test.h"
#include <array>
#include <map>

enum class RenderPassTestMode
{
	None,
	MSAA,
	CopyTexture,
};

void test_renderPass(LLGI::DeviceType deviceType, RenderPassTestMode mode)
{
	auto code_gl_vs = R"(
#version 440 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

out gl_PerVertex
{
	vec4 gl_Position;
};

out vec2 v_uv;
out vec4 v_color;

void main()
{
	gl_Position.x  = a_position.x;
	gl_Position.y  = a_position.y;
	gl_Position.z  = a_position.z;
	gl_Position.w  = 1.0f;
	gl_Position = gl_Position;
	v_uv = a_uv;
	v_color = a_color;
}

)";

	auto code_gl_ps = R"(
#version 440 core
#extension GL_NV_gpu_shader5:require

in vec2 v_uv;
in vec4 v_color;
layout(binding = 0) uniform sampler2D mainTexture;

layout(location = 0) out vec4 color;

void main()
{
    color = v_color * texture(mainTexture, v_uv);
}

)";

	auto code_dx_vs = R"(
struct VS_INPUT{
    float3 Position : POSITION0;
	float2 UV : UV0;
    float4 Color : COLOR0;
};
struct VS_OUTPUT{
    float4 Position : SV_POSITION;
	float2 UV : UV0;
    float4 Color : COLOR0;
};
    
VS_OUTPUT main(VS_INPUT input){
    VS_OUTPUT output;
        
    output.Position = float4(input.Position, 1.0f);
	output.UV = input.UV;
    output.Color = input.Color;
        
    return output;
}
)";

	auto code_dx_ps = R"(
Texture2D txt : register(t0);
SamplerState smp : register(s0);

struct PS_INPUT
{
    float4  Position : SV_POSITION;
	float2  UV : UV0;
    float4  Color    : COLOR0;
};

float4 main(PS_INPUT input) : SV_TARGET 
{ 
	float4 c;
	c = txt.Sample(smp, input.UV);
	c.a = 255;
	return c;
}
)";

	auto compiler = LLGI::CreateCompiler(deviceType);

	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("RenderPass", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreatePlatform(pp, window.get());

	auto graphics = platform->CreateGraphics();
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);

	std::array<LLGI::CommandList*, 3> commandLists;
	for (int i = 0; i < commandLists.size(); i++)
		commandLists[i] = graphics->CreateCommandList(sfMemoryPool);

	LLGI::RenderTextureInitializationParameter params;
	params.Size = LLGI::Vec2I(256, 256);
	params.IsMultiSampling = mode == RenderPassTestMode::MSAA;
	auto renderTexture = graphics->CreateRenderTexture(params);
	assert(renderTexture->GetType() == LLGI::TextureType::Render);

	auto renderTextureDst = graphics->CreateRenderTexture(params);

	auto renderPass = graphics->CreateRenderPass((const LLGI::Texture**)&renderTexture, 1, nullptr);
	assert(renderPass->GetRenderTextureCount() == 1);

	LLGI::TextureInitializationParameter texParam;
	texParam.Size = LLGI::Vec2I(256, 256);
	auto texture = graphics->CreateTexture(texParam);

	auto texture_buf = (LLGI::Color8*)texture->Lock();
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			texture_buf[x + y * 256].R = 128;
			texture_buf[x + y * 256].G = 128;
			texture_buf[x + y * 256].B = 128;
			texture_buf[x + y * 256].A = 255;
		}
	}
	texture->Unlock();

	LLGI::Shader* shader_vs = nullptr;
	LLGI::Shader* shader_ps = nullptr;

	std::vector<LLGI::DataStructure> data_vs;
	std::vector<LLGI::DataStructure> data_ps;

	if (compiler == nullptr)
	{
		auto binary_vs = TestHelper::LoadData("simple_texture_rectangle.vert.spv");
		auto binary_ps = TestHelper::LoadData("simple_texture_rectangle.frag.spv");

		LLGI::DataStructure d_vs;
		LLGI::DataStructure d_ps;

		d_vs.Data = binary_vs.data();
		d_vs.Size = static_cast<int32_t>(binary_vs.size());
		d_ps.Data = binary_ps.data();
		d_ps.Size = static_cast<int32_t>(binary_ps.size());

		data_vs.push_back(d_vs);
		data_ps.push_back(d_ps);

		shader_vs = graphics->CreateShader(data_vs.data(), static_cast<int32_t>(data_vs.size()));
		shader_ps = graphics->CreateShader(data_ps.data(), static_cast<int32_t>(data_ps.size()));
	}
	else
	{
		LLGI::CompilerResult result_vs;
		LLGI::CompilerResult result_ps;

		if (platform->GetDeviceType() == LLGI::DeviceType::Metal)
		{
			auto code_vs = TestHelper::LoadData("simple_texture_rectangle.vert");
			auto code_ps = TestHelper::LoadData("simple_texture_rectangle.frag");
			code_vs.push_back(0);
			code_ps.push_back(0);

			compiler->Compile(result_vs, (const char*)code_vs.data(), LLGI::ShaderStageType::Vertex);
			compiler->Compile(result_ps, (const char*)code_ps.data(), LLGI::ShaderStageType::Pixel);
		}
		else if (platform->GetDeviceType() == LLGI::DeviceType::DirectX12)
		{
			compiler->Compile(result_vs, code_dx_vs, LLGI::ShaderStageType::Vertex);
			assert(result_vs.Message == "");
			compiler->Compile(result_ps, code_dx_ps, LLGI::ShaderStageType::Pixel);
			assert(result_ps.Message == "");
		}
		else
		{
			compiler->Compile(result_vs, code_gl_vs, LLGI::ShaderStageType::Vertex);
			compiler->Compile(result_ps, code_gl_ps, LLGI::ShaderStageType::Pixel);
		}

		std::vector<LLGI::DataStructure> data_vs;
		std::vector<LLGI::DataStructure> data_ps;

		for (auto& b : result_vs.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = static_cast<int32_t>(b.size());
			data_vs.push_back(d);
		}

		for (auto& b : result_ps.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = static_cast<int32_t>(b.size());
			data_ps.push_back(d);
		}

		shader_vs = graphics->CreateShader(data_vs.data(), static_cast<int32_t>(data_vs.size()));
		shader_ps = graphics->CreateShader(data_ps.data(), static_cast<int32_t>(data_ps.size()));
	}

	std::shared_ptr<LLGI::VertexBuffer> vb;
	std::shared_ptr<LLGI::IndexBuffer> ib;
	TestHelper::CreateRectangle(graphics,
								LLGI::Vec3F(-0.5, 0.5, 0.5),
								LLGI::Vec3F(0.5, -0.5, 0.5),
								LLGI::Color8(255, 255, 255, 255),
								LLGI::Color8(0, 255, 0, 255),
								vb,
								ib);

	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pips;

	while (count < 60)
	{
		if (!platform->NewFrame())
		{
			break;
		}

		sfMemoryPool->NewFrame();

		LLGI::Color8 color1;
		color1.R = 0;
		color1.G = count % 255;
		color1.B = 0;
		color1.A = 255;
		renderPass->SetIsColorCleared(true);
		renderPass->SetClearColor(color1);

		LLGI::Color8 color2;
		color2.R = count % 255;
		color2.G = 0;
		color2.B = 0;
		color2.A = 255;

		auto commandList = commandLists[count % commandLists.size()];
		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());

		auto renderPassPipelineState = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPass));

		auto renderPassSc = platform->GetCurrentScreen(color2, true);
		auto renderPassPipelineStateSc = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPassSc));

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
			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs);
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps);
			pip->SetRenderPassPipelineState(renderPassPipelineState.get());
			pip->IsMSAA = mode == RenderPassTestMode::MSAA;
			pip->Compile();

			pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
		}

		// Render to RenderTargetTexture
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->SetTexture(
			texture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->EndRenderPass();

		if (mode == RenderPassTestMode::CopyTexture)
		{
			commandList->CopyTexture(renderTexture, renderTextureDst);
		}

		commandList->BeginRenderPass(platform->GetCurrentScreen(color2, true));
		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());

		if (pips.count(renderPassPipelineStateSc) == 0)
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
			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs);
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps);
			pip->SetRenderPassPipelineState(renderPassPipelineStateSc.get());
			pip->Compile();

			pips[renderPassPipelineStateSc] = LLGI::CreateSharedPtr(pip);
		}

		// Render to backbuffer
		commandList->SetPipelineState(pips[renderPassPipelineStateSc].get());

		if (mode == RenderPassTestMode::CopyTexture)
		{
			commandList->SetTexture(
				renderTextureDst, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		}
		else
		{
			commandList->SetTexture(
				renderTexture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		}

		commandList->Draw(2);
		commandList->EndRenderPass();

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (TestHelper::GetIsCaptureRequired() && count == 30)
		{
			commandList->WaitUntilCompleted();
			auto texture = platform->GetCurrentScreen(LLGI::Color8(), true)->GetRenderTexture(0);
			auto data = graphics->CaptureRenderTarget(texture);

			if (mode == RenderPassTestMode::MSAA)
			{
				Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, texture->GetFormat()).Save("RenderPass.MSAA.png");
			}
			else if (mode == RenderPassTestMode::None)
			{
				Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, texture->GetFormat()).Save("RenderPass.Basic.png");
			}
			else
			{
				Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, texture->GetFormat()).Save("RenderPass.RenderPass.png");
			}
		}
	}

	pips.clear();

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(renderTexture);
	LLGI::SafeRelease(renderTextureDst);
	LLGI::SafeRelease(renderPass);
	LLGI::SafeRelease(texture);
	LLGI::SafeRelease(shader_vs);
	LLGI::SafeRelease(shader_ps);
	for (int i = 0; i < commandLists.size(); i++)
		LLGI::SafeRelease(commandLists[i]);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	LLGI::SafeRelease(compiler);
}

void test_multiRenderPass(LLGI::DeviceType deviceType)
{
	if (deviceType == LLGI::DeviceType::Metal)
		return;

	auto code_gl_vs = R"(
#version 440 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

out gl_PerVertex
{
	vec4 gl_Position;
};

out vec2 v_uv;
out vec4 v_color;

void main()
{
	gl_Position.x  = a_position.x;
	gl_Position.y  = a_position.y;
	gl_Position.z  = a_position.z;
	gl_Position.w  = 1.0f;
	gl_Position = gl_Position;
	v_uv = a_uv;
	v_color = a_color;
}

)";

	auto code_gl_ps = R"(
#version 440 core
#extension GL_NV_gpu_shader5:require

in vec2 v_uv;
in vec4 v_color;
layout(binding = 0) uniform sampler2D mainTexture;

layout(location = 0) out vec4 color;

void main()
{
    color = v_color * texture(mainTexture, v_uv);
}

)";

	auto compiler = LLGI::CreateCompiler(deviceType);

	int count = 0;

	LLGI::PlatformParameter pp;
	pp.Device = deviceType;
	pp.WaitVSync = true;
	auto window = std::unique_ptr<LLGI::Window>(LLGI::CreateWindow("MRT", LLGI::Vec2I(1280, 720)));
	auto platform = LLGI::CreateSharedPtr(LLGI::CreatePlatform(pp, window.get()));

	auto graphics = LLGI::CreateSharedPtr(platform->CreateGraphics());
	auto sfMemoryPool = graphics->CreateSingleFrameMemoryPool(1024 * 1024, 128);

	std::array<LLGI::CommandList*, 3> commandLists;
	for (int i = 0; i < commandLists.size(); i++)
		commandLists[i] = graphics->CreateCommandList(sfMemoryPool);

	LLGI::TextureInitializationParameter texParam;
	texParam.Size = LLGI::Vec2I(256, 256);

	LLGI::RenderTextureInitializationParameter renderTexParam;
	renderTexParam.Size = LLGI::Vec2I(256, 256);

	auto renderTexture1 = graphics->CreateRenderTexture(renderTexParam);
	auto renderTexture2 = graphics->CreateRenderTexture(renderTexParam);
	const LLGI::Texture* renderTextures[2] = {(const LLGI::Texture*)renderTexture1, (const LLGI::Texture*)renderTexture2};
	auto renderPass = graphics->CreateRenderPass((const LLGI::Texture**)renderTextures, 2, nullptr);
	assert(renderPass->GetRenderTextureCount() == 2);

	auto texture = graphics->CreateTexture(texParam);

	auto texture_buf = (LLGI::Color8*)texture->Lock();
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			texture_buf[x + y * 256].R = 255;
			texture_buf[x + y * 256].G = 255;
			texture_buf[x + y * 256].B = 255;
			texture_buf[x + y * 256].A = 255;
		}
	}
	texture->Unlock();

	std::shared_ptr<LLGI::Shader> shader_paste_vs = nullptr;
	std::shared_ptr<LLGI::Shader> shader_paste_ps = nullptr;

	TestHelper::CreateShader(
		graphics.get(), deviceType, "simple_texture_rectangle.vert", "simple_texture_rectangle.frag", shader_paste_vs, shader_paste_ps);

	std::shared_ptr<LLGI::Shader> shader_mrt_vs = nullptr;
	std::shared_ptr<LLGI::Shader> shader_mrt_ps = nullptr;

	TestHelper::CreateShader(
		graphics.get(), deviceType, "simple_texture_rectangle.vert", "simple_mrt_texture_rectangle.frag", shader_mrt_vs, shader_mrt_ps);

	std::shared_ptr<LLGI::VertexBuffer> vb;
	std::shared_ptr<LLGI::IndexBuffer> ib;
	TestHelper::CreateRectangle(graphics.get(),
								LLGI::Vec3F(-0.75, -0.25, 0.5),
								LLGI::Vec3F(-0.25, -0.75, 0.5),
								LLGI::Color8(255, 255, 255, 255),
								LLGI::Color8(0, 255, 0, 255),
								vb,
								ib);

	std::shared_ptr<LLGI::VertexBuffer> vb2;
	std::shared_ptr<LLGI::IndexBuffer> ib2;
	TestHelper::CreateRectangle(graphics.get(),
								LLGI::Vec3F(0.25, 0.75, 0.5),
								LLGI::Vec3F(0.75, 0.25, 0.5),
								LLGI::Color8(255, 255, 255, 255),
								LLGI::Color8(0, 255, 0, 255),
								vb2,
								ib2);

	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pips;

	while (count < 60)
	{
		if (!platform->NewFrame())
		{
			break;
		}

		sfMemoryPool->NewFrame();

		LLGI::Color8 color1;
		color1.R = 0;
		color1.G = count % 255;
		color1.B = 0;
		color1.A = 255;
		renderPass->SetIsColorCleared(true);
		renderPass->SetClearColor(color1);

		LLGI::Color8 color2;
		color2.R = count % 255;
		color2.G = 0;
		color2.B = 0;
		color2.A = 255;

		auto commandList = commandLists[count % commandLists.size()];
		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());

		auto renderPassPipelineState = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPass));

		auto renderPassSc = platform->GetCurrentScreen(color2, true);
		auto renderPassPipelineStateSc = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPassSc));

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
			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_mrt_vs.get());
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_mrt_ps.get());
			pip->SetRenderPassPipelineState(renderPassPipelineState.get());
			pip->Compile();

			pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
		}

		// Render to RenderTargetTexture
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->SetTexture(
			texture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);
		commandList->EndRenderPass();

		commandList->BeginRenderPass(platform->GetCurrentScreen(color2, true));
		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());

		if (pips.count(renderPassPipelineStateSc) == 0)
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
			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_paste_vs.get());
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_paste_ps.get());
			pip->SetRenderPassPipelineState(renderPassPipelineStateSc.get());
			pip->Compile();

			pips[renderPassPipelineStateSc] = LLGI::CreateSharedPtr(pip);
		}

		// Render to Backbuffer
		commandList->SetPipelineState(pips[renderPassPipelineStateSc].get());

		commandList->SetVertexBuffer(vb.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib.get());
		commandList->SetTexture(
			renderTexture1, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->SetVertexBuffer(vb2.get(), sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib2.get());
		commandList->SetTexture(
			renderTexture2, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (TestHelper::GetIsCaptureRequired() && count == 30)
		{
			commandList->WaitUntilCompleted();
			auto texture = platform->GetCurrentScreen(LLGI::Color8(), true)->GetRenderTexture(0);
			auto data = graphics->CaptureRenderTarget(texture);
			Bitmap2D(data, texture->GetSizeAs2D().X, texture->GetSizeAs2D().Y, texture->GetFormat()).Save("RenderPass.MRT.png");
		}
	}

	pips.clear();

	graphics->WaitFinish();

	LLGI::SafeRelease(sfMemoryPool);
	LLGI::SafeRelease(renderTexture1);
	LLGI::SafeRelease(renderTexture2);
	LLGI::SafeRelease(renderPass);
	LLGI::SafeRelease(texture);
	for (int i = 0; i < commandLists.size(); i++)
		LLGI::SafeRelease(commandLists[i]);

	LLGI::SafeRelease(compiler);
}

TestRegister RenderPass_Basic("RenderPass.Basic",
							  [](LLGI::DeviceType device) -> void { test_renderPass(device, RenderPassTestMode::None); });

TestRegister RenderPass_MSAA("RenderPass.MSAA", [](LLGI::DeviceType device) -> void { test_renderPass(device, RenderPassTestMode::MSAA); });

TestRegister RenderPass_CopyTexture("RenderPass.CopyTexture",
									[](LLGI::DeviceType device) -> void { test_renderPass(device, RenderPassTestMode::CopyTexture); });

TestRegister RenderPass_MRT("RenderPass.MRT", [](LLGI::DeviceType device) -> void { test_multiRenderPass(device); });
