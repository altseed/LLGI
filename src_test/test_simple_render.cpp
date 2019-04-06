
#include "test.h"

#include <map>

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

	auto code_dx_vs = R"(
    struct VS_INPUT{
        float3 g_position : POSITION;
        float4 g_color : COLOR;
    };
    struct VS_OUTPUT{
        float4 g_position : SV_POSITION;
        float4 g_color : COLOR;
    };
    
    VS_OUTPUT main(VS_INPUT input){
        VS_OUTPUT output;
        
        output.g_position = float4(input.g_position, 1.0f);
        output.g_color = input.g_color;
        
        return output;
    }
    )";

	auto code_dx_ps = R"(
    struct PS_INPUT{
        float4 Color    : COLOR;
    };
    
    float4 main(PS_INPUT input) : SV_TARGET{
        return input.Color;
    }
    
    )";

	auto code_metal_vs = R"(
    
    struct VertexIn {
        metal::packed_float3 position;
        metal::packed_float2 uv;
        metal::packed_float4 color;
    };
    
    struct VertexOut {
        metal::float4 position [[position]];
        metal::float2 uv;
        metal::float4 color;
    };
    
    vertex VertexOut main(const device VertexIn *vertex_array [[buffer(0)]], unsigned int vid [[vertex_id]]) {
        
        VertexOut vo;
        vo.position = metal::float4(vertex_array[vid].position, 1.0);
        vo.color = vertex_array[vid].color;
        return vo;
    }
    
    )";

	auto code_metal_ps = R"(
    
    
    struct VertexOut {
        metal::float4 position [[position]];
        metal::float2 uv;
        metal::packed_float4 color;
    };
    
    fragment metal::half4 main(VertexOut input [[stage_in]]) {
        return metal::half4(input.color);
    }
    
    )";

	auto compiler = LLGI::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	LLGI::Shader* shader_vs = nullptr;
	LLGI::Shader* shader_ps = nullptr;

	{
		LLGI::CompilerResult result_vs;
		LLGI::CompilerResult result_ps;

		if (compiler->GetDeviceType() == LLGI::DeviceType::DirectX12)
		{
			compiler->Compile(result_vs, code_dx_vs, LLGI::ShaderStageType::Vertex);
			compiler->Compile(result_ps, code_dx_ps, LLGI::ShaderStageType::Pixel);
		}
		else if (compiler->GetDeviceType() == LLGI::DeviceType::Metal)
		{
			compiler->Compile(result_vs, code_metal_vs, LLGI::ShaderStageType::Vertex);
			compiler->Compile(result_ps, code_metal_ps, LLGI::ShaderStageType::Pixel);
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

	vb_buf[0].Color = LLGI::Color8(255, 255, 255, 255);
	vb_buf[1].Color = LLGI::Color8(255, 255, 0, 255);
	vb_buf[2].Color = LLGI::Color8(0, 255, 0, 255);
	vb_buf[3].Color = LLGI::Color8(0, 0, 255, 255);
	vb->Unlock();

	auto ib_buf = (uint16_t*)ib->Lock();
	ib_buf[0] = 0;
	ib_buf[1] = 1;
	ib_buf[2] = 2;
	ib_buf[3] = 0;
	ib_buf[4] = 2;
	ib_buf[5] = 3;
	ib->Unlock();

	std::map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pips;

	while (count < 1000)
	{
		platform->NewFrame();
		graphics->NewFrame();

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		auto renderPass = graphics->GetCurrentScreen(color, true);
		auto renderPassPipelineState = LLGI::CreateSharedPtr(renderPass->CreateRenderPassPipelineState());

		if (pips.count(renderPassPipelineState) == 0)
		{
			auto pip = graphics->CreatePiplineState();
			pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
			pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
			pip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
			pip->VertexLayoutCount = 3;

			pip->SetShader(LLGI::ShaderStageType::Vertex, shader_vs);
			pip->SetShader(LLGI::ShaderStageType::Pixel, shader_ps);
			pip->SetRenderPassPipelineState(renderPassPipelineState.get());
			pip->Compile();

			pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
		}

		commandList->Begin();
		commandList->BeginRenderPass(renderPass);
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pips[renderPassPipelineState].get());
		commandList->Draw(2);
		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;
	}

	pips.clear();

	LLGI::SafeRelease(shader_vs);
	LLGI::SafeRelease(shader_ps);
	LLGI::SafeRelease(ib);
	LLGI::SafeRelease(vb);
	LLGI::SafeRelease(commandList);
	LLGI::SafeRelease(graphics);
	LLGI::SafeRelease(platform);

	LLGI::SafeRelease(compiler);
}

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

	auto compiler = LLGI::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();
	auto texture = graphics->CreateTexture(LLGI::Vec2I(256, 256), false, false);

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

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen(color, true));
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetTexture(
			texture, LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);
		commandList->EndRenderPass();
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

void test_simple_constant_rectangle(LLGI::ConstantBufferType type)
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

	auto compiler = LLGI::CreateCompiler(LLGI::DeviceType::Default);

	int count = 0;

	auto platform = LLGI::CreatePlatform(LLGI::DeviceType::Default);
	auto graphics = platform->CreateGraphics();
	auto commandList = graphics->CreateCommandList();
	auto vb = graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4);
	auto ib = graphics->CreateIndexBuffer(2, 6);
	auto pip = graphics->CreatePiplineState();
	LLGI::ConstantBuffer* cb_vs = nullptr;
	LLGI::ConstantBuffer* cb_ps = nullptr;

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

	vb_buf[0].Color = LLGI::Color8(255, 255, 255, 255);
	vb_buf[1].Color = LLGI::Color8(255, 255, 0, 255);
	vb_buf[2].Color = LLGI::Color8(0, 255, 0, 255);
	vb_buf[3].Color = LLGI::Color8(0, 0, 255, 255);

	vb->Unlock();

	auto ib_buf = (uint16_t*)ib->Lock();
	ib_buf[0] = 0;
	ib_buf[1] = 1;
	ib_buf[2] = 2;
	ib_buf[3] = 0;
	ib_buf[4] = 2;
	ib_buf[5] = 3;
	ib->Unlock();

	if (type == LLGI::ConstantBufferType::LongTime)
	{
		cb_vs = graphics->CreateConstantBuffer(sizeof(float) * 4);
		cb_ps = graphics->CreateConstantBuffer(sizeof(float) * 4);

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
	}

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

		if (type == LLGI::ConstantBufferType::ShortTime)
		{
			cb_vs = graphics->CreateConstantBuffer(sizeof(float) * 4, type);
			cb_ps = graphics->CreateConstantBuffer(sizeof(float) * 4, type);

			auto cb_vs_buf = (float*)cb_vs->Lock();
			cb_vs_buf[0] = (count % 100) / 100.0f;
			cb_vs_buf[1] = 0.0f;
			cb_vs_buf[2] = 0.0f;
			cb_vs_buf[3] = 0.0f;

			auto cb_ps_buf = (float*)cb_ps->Lock();
			cb_ps_buf[0] = 0.0f;
			cb_ps_buf[1] = -1.0f;
			cb_ps_buf[2] = -1.0f;
			cb_ps_buf[3] = 0.0f;
		}

		LLGI::Color8 color;
		color.R = count % 255;
		color.G = 0;
		color.B = 0;
		color.A = 255;

		commandList->Begin();
		commandList->BeginRenderPass(graphics->GetCurrentScreen(color, true));
		commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), 0);
		commandList->SetIndexBuffer(ib);
		commandList->SetPipelineState(pip);
		commandList->SetConstantBuffer(cb_vs, LLGI::ShaderStageType::Vertex);
		commandList->SetConstantBuffer(cb_ps, LLGI::ShaderStageType::Pixel);
		commandList->Draw(2);

		commandList->EndRenderPass();
		commandList->End();

		graphics->Execute(commandList);

		platform->Present();
		count++;

		if (type == LLGI::ConstantBufferType::ShortTime)
		{
			LLGI::SafeRelease(cb_vs);
			LLGI::SafeRelease(cb_ps);
		}
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
