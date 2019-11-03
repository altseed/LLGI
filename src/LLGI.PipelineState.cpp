
#include "LLGI.PipelineState.h"
#include "LLGI.Graphics.h"

namespace LLGI
{

void PipelineState::SetShader(ShaderStageType stage, Shader* shader) {}

void PipelineState::SetRenderPassPipelineState(RenderPassPipelineState* renderPassPipelineState)
{
	VertexLayoutSemantics.fill(0);

	SafeAddRef(renderPassPipelineState);
	renderPassPipelineState_ = CreateSharedPtr(renderPassPipelineState);
}

void PipelineState::Compile() {}

} // namespace LLGI