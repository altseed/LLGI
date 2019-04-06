
#include "LLGI.PipelineState.h"
#include "LLGI.Graphics.h"

namespace LLGI
{

void PipelineState::SetShader(ShaderStageType stage, Shader* shader) {}

void PipelineState::SetRenderPassPipelineState(RenderPassPipelineState* renderPassPipelineState) {

	SafeAddRef(renderPassPipelineState);
	renderPassPipelineState_ = CreateSharedPtr(renderPassPipelineState);
}

void PipelineState::Compile() {}

} // namespace LLGI