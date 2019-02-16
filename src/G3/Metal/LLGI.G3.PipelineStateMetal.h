#pragma once

#include "../LLGI.G3.PipelineState.h"

namespace LLGI
{
namespace G3
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

} // namespace G3
} // namespace LLGI
