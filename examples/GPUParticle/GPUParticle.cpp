
#include <LLGI.CommandList.h>
#include <LLGI.Graphics.h>
#include <LLGI.Platform.h>
#include <LLGI.Shader.h>
#include <LLGI.Compiler.h>
#include <LLGI.VertexBuffer.h>
#include <LLGI.IndexBuffer.h>
#include <LLGI.PipelineState.h>
#include <LLGI.Texture.h>
#include <LLGI.ConstantBuffer.h>
#include "GPUParticle.h"

//==============================================================================
// GPUParticleEmitPass

Shader::Shader(
	LLGI::Graphics* graphics,
	LLGI::DeviceType deviceType,
	const char* vsBinaryPath,
	const char* psBinaryPath)
{

	auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(deviceType));

	std::vector<LLGI::DataStructure> data_vs;
	std::vector<LLGI::DataStructure> data_ps;

	if (compiler == nullptr)
	{
		auto vsBinaryPath_ = std::string(vsBinaryPath);
		auto psBinaryPath_ = std::string(psBinaryPath);

		// if (deviceType == LLGI::DeviceType::Vulkan)
		{
			vsBinaryPath_ += ".spv";
			psBinaryPath_ += ".spv";
		}

		auto binary_vs = LoadData(vsBinaryPath_.c_str());
		auto binary_ps = LoadData(psBinaryPath_.c_str());

		LLGI::DataStructure d_vs;
		LLGI::DataStructure d_ps;

		d_vs.Data = binary_vs.data();
		d_vs.Size = static_cast<int32_t>(binary_vs.size());
		d_ps.Data = binary_ps.data();
		d_ps.Size = static_cast<int32_t>(binary_ps.size());

		data_vs.push_back(d_vs);
		data_ps.push_back(d_ps);

		m_vs = LLGI::CreateSharedPtr(graphics->CreateShader(data_vs.data(), static_cast<int32_t>(data_vs.size())));
		m_ps = LLGI::CreateSharedPtr(graphics->CreateShader(data_ps.data(), static_cast<int32_t>(data_ps.size())));
	}
	else
	{
		LLGI::CompilerResult result_vs;
		LLGI::CompilerResult result_ps;

		auto vsBinaryPath_ = std::string(vsBinaryPath);
		auto psBinaryPath_ = std::string(psBinaryPath);

		auto code_vs = LoadData(vsBinaryPath_.c_str());
		auto code_ps = LoadData(psBinaryPath_.c_str());
		code_vs.push_back(0);
		code_ps.push_back(0);

		compiler->Compile(result_vs, (const char*)code_vs.data(), LLGI::ShaderStageType::Vertex);
		compiler->Compile(result_ps, (const char*)code_ps.data(), LLGI::ShaderStageType::Pixel);

		std::cout << result_vs.Message.c_str() << std::endl;
		std::cout << result_ps.Message.c_str() << std::endl;

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

		m_vs = LLGI::CreateSharedPtr(graphics->CreateShader(data_vs.data(), static_cast<int32_t>(data_vs.size())));
		m_ps = LLGI::CreateSharedPtr(graphics->CreateShader(data_ps.data(), static_cast<int32_t>(data_ps.size())));
	}
}

std::vector<uint8_t> Shader::LoadData(const char* path)
{
	std::vector<uint8_t> ret;

#ifdef _WIN32
	FILE* fp = nullptr;
	fopen_s(&fp, path, "rb");

#else
	FILE* fp = fopen(path, "rb");
#endif

	if (fp == nullptr)
	{
		std::cout << "Not found : " << path << std::endl;
		return ret;
	}

	fseek(fp, 0, SEEK_END);
	auto size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ret.resize(size);
	fread(ret.data(), 1, size, fp);
	fclose(fp);

	return ret;
}

//==============================================================================
// GPUParticleEmitPass

GPUParticleEmitPass::GPUParticleEmitPass(GPUParticleContext* context)
	: context_(context)
{
	shader_ = std::make_unique<Shader>(context_->GetGraphcis(), context_->GetDeviceType(),
		"C:/Proj/LN/Lumino/build/ExternalSource/Effekseer/Dev/Cpp/3rdParty/LLGI/examples/GPUParticle/Shaders/HLSL_DX12/perticle-emit.vert",
		"C:/Proj/LN/Lumino/build/ExternalSource/Effekseer/Dev/Cpp/3rdParty/LLGI/examples/GPUParticle/Shaders/HLSL_DX12/perticle-emit.frag");


	for (int i = 0; i < context_->GetMaxFrameCount(); i++) {
		auto vb = LLGI::CreateSharedPtr(context_->GetGraphcis()->CreateVertexBuffer(sizeof(EmitDataVertex) * context_->GetMaxParticles()));
		emitDataVertexBuffer_.push_back(vb);
	}

	emitDataIndexBuffer_ = LLGI::CreateSharedPtr(context_->GetGraphcis()->CreateIndexBuffer(sizeof(uint32_t), context_->GetMaxParticles()));
	auto ib_buf = (uint32_t*)emitDataIndexBuffer_->Lock();
	for (int i = 0; i < context_->GetMaxParticles(); i++) {
		ib_buf[i] = i;
	}
	emitDataIndexBuffer_->Unlock();

	LLGI::Texture* textures[] = { context_->GetPositionTexture(), context_->GetVelocityTexture() };
	emitParticleRenderPass_ = LLGI::CreateSharedPtr(context_->GetGraphcis()->CreateRenderPass(textures, 2, nullptr));

	pipelineState_ = LLGI::CreateSharedPtr(context_->GetGraphcis()->CreatePiplineState());
	pipelineState_->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
	pipelineState_->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
	pipelineState_->VertexLayouts[2] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
	pipelineState_->VertexLayoutNames[0] = "UV";
	pipelineState_->VertexLayoutNames[1] = "POSITION";
	pipelineState_->VertexLayoutNames[2] = "COLOR";
	pipelineState_->VertexLayoutCount = 3;
	pipelineState_->Topology = LLGI::TopologyType::Point;
	pipelineState_->SetShader(LLGI::ShaderStageType::Vertex, shader_->vertexShader());
	pipelineState_->SetShader(LLGI::ShaderStageType::Pixel, shader_->pixelShader());
	pipelineState_->SetRenderPassPipelineState(context_->GetGraphcis()->CreateRenderPassPipelineState(emitParticleRenderPass_.get()));
	pipelineState_->Compile();

	textureInfoConstantBuffer_ = LLGI::CreateSharedPtr(context_->GetGraphcis()->CreateConstantBuffer(sizeof(GPUParticleTextureInfo)));
	auto info = (GPUParticleTextureInfo*)textureInfoConstantBuffer_->Lock();
	info->TextureSize[0] = context_->GetBufferTextureWidth();
	info->TextureSize[1] = context_->GetBufferTextureWidth();
	info->TextureSize[2] = 0;
	info->TextureSize[3] = 0;
	textureInfoConstantBuffer_->Unlock();
}

void GPUParticleEmitPass::Render(LLGI::CommandList* commandList, const std::vector<EmitDataVertex>& emitData, int particleDataCount)
{
	// Apply new particles
	auto currentVB = emitDataVertexBuffer_[context_->GetFrameIndex()].get();
	{
		auto vb_buf = (EmitDataVertex*)currentVB->Lock();
		memcpy(vb_buf, emitData.data(), sizeof(EmitDataVertex) * particleDataCount);
		currentVB->Unlock();
	}


	commandList->BeginRenderPass(emitParticleRenderPass_.get());

	commandList->SetConstantBuffer(textureInfoConstantBuffer_.get(), LLGI::ShaderStageType::Vertex);
	commandList->SetVertexBuffer(currentVB, sizeof(EmitDataVertex), 0);
	commandList->SetIndexBuffer(emitDataIndexBuffer_.get());
	commandList->SetPipelineState(pipelineState_.get());
	commandList->Draw(particleDataCount);

	commandList->EndRenderPass();
}

//==============================================================================
// GPUParticleContext

GPUParticleContext::GPUParticleContext(LLGI::Graphics* graphics, LLGI::DeviceType deviceType, int frameCount, int textureSize)
	: graphcis_(graphics)
	, deviceType_(deviceType)
	, bufferTextureWidth_(textureSize)
	, maxTexels_(textureSize* textureSize)
	, frameIndex_(-1)
	, maxFrameCount_(frameCount)
	, emitedCount_(0)
	, newParticleCountInFrame_(0)
{
	emitData_.resize(kMaxOneFrameEmitCount);

	LLGI::RenderTextureInitializationParameter params;
	params.Size = LLGI::Vec2I(textureSize, textureSize);
	params.Format = LLGI::TextureFormatType::R32G32B32A32_FLOAT;
	positionTexture_ = LLGI::CreateSharedPtr(graphics->CreateRenderTexture(params));
	velocityTexture_ = LLGI::CreateSharedPtr(graphics->CreateRenderTexture(params));

	particleEmitPass_ = std::make_unique<GPUParticleEmitPass>(this);
}

void GPUParticleContext::Emit(float lifeTime, LLGI::Vec3F position, LLGI::Vec3F velocity)
{
	int index = newParticleCountInFrame_;
	emitData_[index].ParticleIdAndLifeTime = LLGI::Vec2F((emitedCount_ + newParticleCountInFrame_) % maxTexels_, lifeTime);
	emitData_[index].Position = position;
	emitData_[index].Velocity = velocity;
	newParticleCountInFrame_++;
}

void GPUParticleContext::NewFrame()
{
	frameIndex_ = (frameIndex_ + 1) % maxFrameCount_;
}

void GPUParticleContext::Render(LLGI::RenderPass* renderPass, LLGI::CommandList* commandList)
{
	particleEmitPass_->Render(commandList, emitData_, newParticleCountInFrame_);
}
