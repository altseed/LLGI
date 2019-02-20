
#pragma once

#include "../LLGI.G3.PipelineState.h"
#include "../LLGI.G3.Shader.h"
#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{
namespace G3
{

class PipelineStateDX12 : public PipelineState
{
private:
	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> pixelShader;

public:
	PipelineStateDX12() = default;
	virtual ~PipelineStateDX12();

	bool Initialize();

	void SetShader(ShaderStageType stage, Shader* shader) override;
	void Compile() override;
};

} // namespace G3
} // namespace LLGI
