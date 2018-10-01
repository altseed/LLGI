
#include <G3/LLGI.G3.Platform.h>
#include <G3/LLGI.G3.Graphics.h>
#include <G3/LLGI.G3.CommandList.h>
#include <G3/LLGI.G3.VertexBuffer.h>
#include <G3/LLGI.G3.IndexBuffer.h>
#include <G3/LLGI.G3.Compiler.h>
#include <G3/LLGI.G3.Shader.h>
#include <G3/LLGI.G3.PipelineState.h>
#include <G3/LLGI.G3.ConstantBuffer.h>
#include <G3/LLGI.G3.Texture.h>

#include <iostream>

struct SimpleVertex
{
	float Pos[3];
	float UV[2];
	LLGI::Color8 Color;
};

void test_simple_texture_rectangle()
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

	auto compiler = LLGI::G3::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();
	auto texture = graphics->CreateTexture(LLGI::Vec2I(256, 256));

	auto texture_buf = (LLGI::Color8*)texture->Lock();
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			texture_buf[x + y * 256].R = 255;
			texture_buf[x + y * 256].G = y;
			texture_buf[x + y * 256].B = 255;
			texture_buf[x + y * 256].A = 255;
		}
	}
	texture->Unlock();

	LLGI::G3::Shader* shader_vs = nullptr;
	LLGI::G3::Shader* shader_ps = nullptr;

	{
		LLGI::G3::CompilerResult result_vs;
		LLGI::G3::CompilerResult result_ps;

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
	vb_buf[0].Pos[0] = -0.5;
	vb_buf[0].Pos[1] = 0.5;
	vb_buf[0].Pos[2] = 0.5;

	vb_buf[1].Pos[0] = 0.5;
	vb_buf[1].Pos[1] = 0.5;
	vb_buf[1].Pos[2] = 0.5;

	vb_buf[2].Pos[0] = 0.5;
	vb_buf[2].Pos[1] = -0.5;
	vb_buf[2].Pos[2] = 0.5;

	vb_buf[3].Pos[0] = -0.5;
	vb_buf[3].Pos[1] = -0.5;
	vb_buf[3].Pos[2] = 0.5;

	vb_buf[0].UV[0] = 0.0;
	vb_buf[0].UV[1] = 0.0;

	vb_buf[1].UV[0] = 1.0;
	vb_buf[1].UV[1] = 0.0;

	vb_buf[2].UV[0] = 1.0;
	vb_buf[2].UV[1] = 1.0;

	vb_buf[3].UV[0] = 0.0;
	vb_buf[3].UV[1] = 1.0;

	vb_buf[0].Color.R = 255;
	vb_buf[0].Color.G = 255;
	vb_buf[0].Color.B = 255;
	vb_buf[0].Color.A = 255;

	vb_buf[1].Color = vb_buf[0].Color;
	vb_buf[2].Color = vb_buf[0].Color;
	vb_buf[3].Color = vb_buf[0].Color;

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

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(graphics->GetCurrentScreen(), color);
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex));
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetTexture(texture, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

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

void test_simple_constant_rectangle()
{
	auto code_gl_vs = R"(
#version 440 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

layout(binding = 0) uniform Block 
{
	vec4 u_offset;
};

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
	gl_Position = gl_Position + u_offset;
	v_uv = a_uv;
	v_color = a_color;
}

)";

	auto code_gl_ps = R"(
#version 440 core
#extension GL_NV_gpu_shader5:require

layout(binding = 0) uniform Block 
{
	vec4 u_offset;
};

in vec2 v_uv;
in vec4 v_color;

layout(location = 0) out vec4 color;

void main()
{
    color = v_color + u_offset;
}

)";

	auto compiler = LLGI::G3::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();
	auto cb_vs = graphics->CreateConstantBuffer(sizeof(float) * 4);
	auto cb_ps = graphics->CreateConstantBuffer(sizeof(float) * 4);

	LLGI::G3::Shader* shader_vs = nullptr;
	LLGI::G3::Shader* shader_ps = nullptr;

	{
		LLGI::G3::CompilerResult result_vs;
		LLGI::G3::CompilerResult result_ps;

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
	vb_buf[0].Pos[0] = -0.5;
	vb_buf[0].Pos[1] = 0.5;
	vb_buf[0].Pos[2] = 0.5;

	vb_buf[1].Pos[0] = 0.5;
	vb_buf[1].Pos[1] = 0.5;
	vb_buf[1].Pos[2] = 0.5;

	vb_buf[2].Pos[0] = 0.5;
	vb_buf[2].Pos[1] = -0.5;
	vb_buf[2].Pos[2] = 0.5;

	vb_buf[3].Pos[0] = -0.5;
	vb_buf[3].Pos[1] = -0.5;
	vb_buf[3].Pos[2] = 0.5;

	vb_buf[0].Color.R = 255;
	vb_buf[0].Color.G = 255;
	vb_buf[0].Color.B = 255;
	vb_buf[0].Color.A = 255;

	vb_buf[1].Color.R = 255;
	vb_buf[1].Color.G = 255;
	vb_buf[1].Color.B = 0;
	vb_buf[1].Color.A = 255;

	vb_buf[2].Color.R = 0;
	vb_buf[2].Color.G = 255;
	vb_buf[2].Color.B = 0;
	vb_buf[2].Color.A = 255;

	vb_buf[3].Color.R = 0;
	vb_buf[3].Color.G = 0;
	vb_buf[3].Color.B = 255;
	vb_buf[3].Color.A = 255;

	vb->Unlock();

	auto ib_buf = (uint16_t*)ib->Lock();
	ib_buf[0] = 0;
	ib_buf[1] = 1;
	ib_buf[2] = 2;
	ib_buf[3] = 0;
	ib_buf[4] = 2;
	ib_buf[5] = 3;
	ib->Unlock();

	auto cb_vs_buf = (float*)cb_vs->Lock();
	cb_vs_buf[0] = 0.2f;
	cb_vs_buf[1] = 0.0f;
	cb_vs_buf[2] = 0.0f;
	cb_vs_buf[3] = 0.0f;

	auto cb_ps_buf = (float*)cb_ps->Lock();
	cb_ps_buf[0] = 0.0f;
	cb_ps_buf[1] = -1.0f;
	cb_ps_buf[2] = -1.0f;
	cb_ps_buf[3] = 0.0f;

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

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(graphics->GetCurrentScreen(), color);
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex));
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetConstantBuffer(cb_vs, LLGI::ShaderStageType::Vertex);
		commandList->SetConstantBuffer(cb_ps, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(cb_vs);
	LLGI::SafeRelease(cb_ps);
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

void test_simple_rectangle()
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
	v_uv = a_uv;
	v_color = a_color;
}

)";

	auto code_gl_ps = R"(
#version 440 core
#extension GL_NV_gpu_shader5:require

in vec2 v_uv;
in vec4 v_color;

layout(location = 0) out vec4 color;

void main()
{
    color = v_color;
}

)";

	auto compiler = LLGI::G3::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();
	LLGI::G3::Shader* shader_vs = nullptr;
	LLGI::G3::Shader* shader_ps = nullptr;

	{
		LLGI::G3::CompilerResult result_vs;
		LLGI::G3::CompilerResult result_ps;

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
	vb_buf[0].Pos[0] = -0.5;
	vb_buf[0].Pos[1] = 0.5;
	vb_buf[0].Pos[2] = 0.5;

	vb_buf[1].Pos[0] = 0.5;
	vb_buf[1].Pos[1] = 0.5;
	vb_buf[1].Pos[2] = 0.5;

	vb_buf[2].Pos[0] = 0.5;
	vb_buf[2].Pos[1] = -0.5;
	vb_buf[2].Pos[2] = 0.5;

	vb_buf[3].Pos[0] = -0.5;
	vb_buf[3].Pos[1] = -0.5;
	vb_buf[3].Pos[2] = 0.5;

	vb_buf[0].Color.R = 255;
	vb_buf[0].Color.G = 255;
	vb_buf[0].Color.B = 255;
	vb_buf[0].Color.A = 255;

	vb_buf[1].Color.R = 255;
	vb_buf[1].Color.G = 255;
	vb_buf[1].Color.B = 0;
	vb_buf[1].Color.A = 255;

	vb_buf[2].Color.R = 0;
	vb_buf[2].Color.G = 255;
	vb_buf[2].Color.B = 0;
	vb_buf[2].Color.A = 255;

	vb_buf[3].Color.R = 0;
	vb_buf[3].Color.G = 0;
	vb_buf[3].Color.B = 255;
	vb_buf[3].Color.A = 255;

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
	pip->SetShader(LLGI::ShaderStageType::Pixel , shader_ps);
	pip->Compile();

	while (count < 1000)
	{
		platform->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(graphics->GetCurrentScreen(), color);
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex));
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->Draw(2);

		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

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

void test_compile()
{
	auto compiler = LLGI::G3::CreateCompiler(LLGI::DeviceType::Default);

	LLGI::G3::CompilerResult result_vs;
	LLGI::G3::CompilerResult result_ps;

	auto code_vs = R"(
#version 440 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec2 v_uv;

void main()
{
	gl_Position.x  = a_position.x;
	gl_Position.y  = a_position.y;
	gl_Position.z  = a_position.z;
	gl_Position.w  = 1.0f;
	v_uv  = a_uv;
}

)";

	auto code_ps = R"(
#version 440 core
#extension GL_NV_gpu_shader5:require

in vec2 v_uv;

layout(location = 0) out vec4 color;

void main()
{
   color  = vec4(1.0, 1.0, 1.0, 1.0);
}

)";

	compiler->Compile(result_vs, code_vs, LLGI::ShaderStageType::Vertex);
	compiler->Compile(result_ps, code_ps, LLGI::ShaderStageType::Pixel);

	std::cout << result_vs.Message.c_str() << std::endl;
	std::cout << result_ps.Message.c_str() << std::endl;

	LLGI::SafeRelease(compiler);
}

void test_clear_update()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	while (count < 1000)
	{
		platform->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->SetScissor(0, 0, 1280, 720);
		commandList->Clear(graphics->GetCurrentScreen(), color);
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

void test_clear()
{
	int count = 0;

	auto platform = LLGI::G3::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();

	LLGI::Color8 color;
	color.R = 255;
	color.G = 0;
	color.B = 0;
	color.A = 255;

	commandList->Begin();
	commandList->SetScissor(0, 0, 1280, 720);
	commandList->Clear(graphics->GetCurrentScreen(), color);
	commandList->End();

	while (count < 1000)
	{
		platform->NewFrame();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);
}

int main()
{
	test_simple_texture_rectangle();
	//test_simple_constant_rectangle();
	//test_simple_rectangle();
	//test_compile();
	//test_clear_update();
	//test_clear();


	return 0;
}
