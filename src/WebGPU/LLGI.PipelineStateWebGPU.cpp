#include "LLGI.PipelineStateWebGPU.h"

namespace LLGI
{

PipelineStateWebGPU::PipelineStateWebGPU() { shaders_.fill(nullptr); }

PipelineStateWebGPU::~PipelineStateWebGPU()
{
	for (auto& shader : shaders_)
	{
		SafeRelease(shader);
	}
}

void PipelineStateWebGPU::SetShader(ShaderStageType stage, Shader* shader)
{
	SafeAddRef(shader);
	SafeRelease(shaders_[static_cast<int>(stage)]);
	shaders_[static_cast<int>(stage)] = shader;
}

void PipelineStateWebGPU::SetRenderPassPipelineState(RenderPassPipelineState* renderPassPipelineState) { throw "Unimpleneted"; }

bool PipelineStateWebGPU::Compile()
{
	wgpu::RenderPipelineDescriptor desc{};

	throw "Unimpleneted";
	return false;
}

} // namespace LLGI