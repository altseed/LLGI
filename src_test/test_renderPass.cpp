#include "test.h"

void test_renderPass()
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

	auto compiler = LLGI::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();

	auto renderTexture = graphics->CreateTexture(LLGI::Vec2I(256, 256), true, false);

	auto renderPass = graphics->CreateRenderPass((const LLGI::Texture**)&renderTexture, 1, nullptr);

	auto texture = graphics->CreateTexture(LLGI::Vec2I(256, 256), false, false);

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

	LLGI::Shader* shader_vs = nullptr;
	LLGI::Shader* shader_ps = nullptr;

	{
		LLGI::CompilerResult result_vs;
		LLGI::CompilerResult result_ps;

		compiler->Compile(result_vs, code_gl_vs, LLGI::ShaderStageType::Vertex);
		compiler->Compile(result_ps, code_gl_ps, LLGI::ShaderStageType::Pixel);

		std::vector<LLGI::DataStructure> data_vs;
		std::vector<LLGI::DataStructure> data_ps;

		for (auto& b : result_vs.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = b.size();
			data_vs.push_back(d);
		}

		for (auto& b : result_ps.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = b.size();
			data_ps.push_back(d);
		}

		shader_vs = graphics->CreateShader(data_vs.data(), data_vs.size());
		shader_ps = graphics->CreateShader(data_ps.data(), data_ps.size());
	}

	auto vb_buf = (SimpleVertex*)vb->Lock();
	vb_buf[0].Pos = LLGI::Vec3F(-0.5, 0.5, 0.5);
	vb_buf[1].Pos = LLGI::Vec3F(0.5, 0.5, 0.5);
	vb_buf[2].Pos = LLGI::Vec3F(0.5, -0.5, 0.5);
	vb_buf[3].Pos = LLGI::Vec3F(-0.5, -0.5, 0.5);

	vb_buf[0].UV = LLGI::Vec2F(0.0f, 0.0f);
	vb_buf[1].UV = LLGI::Vec2F(1.0f, 0.0f);
	vb_buf[2].UV = LLGI::Vec2F(1.0f, 1.0f);
	vb_buf[3].UV = LLGI::Vec2F(0.0f, 1.0f);

	vb_buf[0].Color = LLGI::Color8();
	vb_buf[1].Color = LLGI::Color8();
	vb_buf[2].Color = LLGI::Color8();
	vb_buf[3].Color = LLGI::Color8();

	vb->Unlock();

	auto ib_buf = (uint16_t*)ib->Lock();
	ib_buf[0] = 0;
	ib_buf[1] = 1;
	ib_buf[2] = 2;
	ib_buf[3] = 0;
	ib_buf[4] = 2;
	ib_buf[5] = 3;
	ib->Unlock();

	pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
	pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
	pip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
	pip->VertexLayoutCount = 3;

	pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs);
	pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps);
	pip->Compile();

	while (count < 1000)
	{
		platform->NewFrame();
		graphics->NewFrame();

		LLGI::Color8 color1;
		color1.R = 0;
		color1.G = count % 255;
		color1.B = 0;
		color1.A = 255;
		renderPass->SetIsColorCleared(true);
		renderPass->SetClearColor(color1);

		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetTexture(
			texture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);
		commandList->EndRenderPass();

		LLGI::Color8 color2;
		color2.R = count % 255;
		color2.G = 0;
		color2.B = 0;
		color2.A = 255;

		commandList->BeginRenderPass(graphics->GetCurrentScreen(color2, true));
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetTexture(
			renderTexture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);
		commandList->EndRenderPass();

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(renderTexture);
	LLGI::SafeRelease(renderPass);
	LLGI::SafeRelease(texture);
	LLGI::SafeRelease(shader_vs);
	LLGI::SafeRelease(shader_ps);
	LLGI::SafeRelease(pip);
	LLGI::SafeRelease(ib);
	LLGI::SafeRelease(vb);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	LLGI::SafeRelease(compiler);
}
