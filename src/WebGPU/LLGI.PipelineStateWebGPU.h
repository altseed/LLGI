#pragma once

#include "../LLGI.PipelineState.h"
#include "LLGI.BaseWebGPU.h"

namespace LLGI
{

class PipelineStateWebGPU : public PipelineState
{
	std::array<Shader*, static_cast<int>(ShaderStageType::Max)> shaders_;

    wgpu::RenderPipeline renderPipeline_;

public:
	PipelineStateWebGPU();
	~PipelineStateWebGPU() override;
	
	void SetShader(ShaderStageType stage, Shader* shader) override;

	void SetRenderPassPipelineState(RenderPassPipelineState* renderPassPipelineState) override;

	bool Compile() override;
};

}