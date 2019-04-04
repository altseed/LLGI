#pragma once

#include "../LLGI.PipelineState.h"

namespace LLGI
{
    
struct PipelineState_Impl;

class PipelineStateMetal : public PipelineState
{
private:
	PipelineState_Impl* impl = nullptr;

public:
	PipelineStateMetal() = default;
	virtual ~PipelineStateMetal() = default;

	void SetShader(ShaderStageType stage, Shader* shader) override;
	void Compile() override;
};

} // namespace LLGI
