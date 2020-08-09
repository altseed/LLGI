#pragma once
#include <memory>
#include <iostream>
#include <unordered_map>

class Shader;
class GPUParticleEmitPass;
class GPUParticleRenderPass;
class GPUParticleContext;

struct EmitDataVertex
{
	LLGI::Vec2F ParticleIdAndLifeTime;
	LLGI::Vec3F Position;
	LLGI::Vec3F Velocity;
};

struct alignas(16) GPUParticleTextureInfo
{
	float TextureSize[4];
};

class Shader
{
public:
	Shader(
		LLGI::Graphics* graphics,
		LLGI::DeviceType deviceType,
		const char* vsBinaryPath,
		const char* psBinaryPath);

	LLGI::Shader* vertexShader() const { return m_vs.get(); }
	LLGI::Shader* pixelShader() const { return m_ps.get(); }

private:
	std::vector<uint8_t> LoadData(const char* path);

	std::shared_ptr<LLGI::Shader> m_vs;
	std::shared_ptr<LLGI::Shader> m_ps;
};

class GPUParticleEmitPass
{
public:
	GPUParticleEmitPass(GPUParticleContext* context);

	void Render(LLGI::CommandList* commandList, const std::vector<EmitDataVertex>& emitData, int particleDataCount);

private:
	GPUParticleContext* context_;
	std::unique_ptr<Shader> shader_;

	std::vector<std::shared_ptr<LLGI::VertexBuffer>> emitDataVertexBuffer_;	// LLGI::CommandList::SetData() が未対応なので、Lock/Unlock で対応することになるが、そのため FrameCount 分必要

	// いまは DX12 バックエンドが IndexBuffer 必須となっているので用意する必要がある
	std::shared_ptr<LLGI::IndexBuffer> emitDataIndexBuffer_;


	std::shared_ptr<LLGI::RenderPass> emitParticleRenderPass_;
	std::shared_ptr<LLGI::PipelineState> pipelineState_;
	std::shared_ptr<LLGI::ConstantBuffer> textureInfoConstantBuffer_;


	std::unordered_map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pipelineCache_;
};

class GPUParticleContext
{
public:
	static const int kMaxOneFrameEmitCount = 1024;

	GPUParticleContext(LLGI::Graphics* graphics, LLGI::DeviceType deviceType, int frameCount, int textureSize);

	LLGI::Graphics* GetGraphcis() const { return graphcis_; }

	LLGI::DeviceType GetDeviceType() const { return deviceType_; }

	int GetFrameIndex() const { return frameIndex_; }

	int GetMaxFrameCount() const { return maxFrameCount_; }

	int GetMaxParticles() const { return maxTexels_; }

	int GetBufferTextureWidth() const { return bufferTextureWidth_; }

	LLGI::Texture* GetPositionTexture() const { return positionTexture_.get(); }

	LLGI::Texture* GetVelocityTexture() const { return velocityTexture_.get(); }

	void Emit(float lifeTime, LLGI::Vec3F position, LLGI::Vec3F velocity);

	void NewFrame();

	void Render(LLGI::RenderPass* renderPass, LLGI::CommandList* commandList);

private:
	LLGI::Graphics* graphcis_;
	LLGI::DeviceType deviceType_;
	int frameIndex_;
	int maxFrameCount_;
	int bufferTextureWidth_;
	int maxTexels_;

	std::vector<EmitDataVertex> emitData_;
	int emitedCount_;
	int newParticleCountInFrame_;

	std::shared_ptr<LLGI::Texture> positionTexture_;
	std::shared_ptr<LLGI::Texture> velocityTexture_;

	std::unique_ptr<GPUParticleEmitPass> particleEmitPass_;
};


