
#pragma once

#include "../LLGI.PipelineState.h"
#include "../LLGI.Shader.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{

class PipelineStateDX12 : public PipelineState
{
private:
	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> pixelShader;

	ID3D12Resource* pipelineState_ = nullptr;
	std::shared_ptr<GraphicsDX12> graphics_;

public:
	PipelineStateDX12() = default;
	PipelineStateDX12(GraphicsDX12* graphics);
	virtual ~PipelineStateDX12();

	void SetShader(ShaderStageType stage, Shader* shader) override;
	void Compile() override;
};

} // namespace LLGI
