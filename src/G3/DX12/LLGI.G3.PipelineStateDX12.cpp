
#include "LLGI.G3.PipelineStateDX12.h"
#include "../LLGI.G3.PipelineState.h"
#include "LLGI.G3.CompilerDX12.h"

namespace LLGI
{
namespace G3
{
PipelineStateDX12::~PipelineStateDX12()
{
	SafeRelease(pixelShader);
	SafeRelease(vertexShader);
}
void PipelineStateDX12::SetShader(ShaderStageType stage, Shader* shader)
{
	switch (stage)
	{
	case ShaderStageType::Pixel:
		SafeAddRef(shader);
		pixelShader = CreateSharedPtr(shader);
		break;
	case ShaderStageType::Vertex:
		SafeAddRef(shader);
		vertexShader = CreateSharedPtr(shader);
		break;
	}
}

void PipelineStateDX12::Compile() {}

} // namespace G3
} // namespace LLGI
