#pragma once

#include "../LLGI.PipelineState.h"

namespace LLGI
{

class GraphicsMetal;
struct PipelineState_Impl;

class PipelineStateMetal : public PipelineState
{
private:
	GraphicsMetal* graphics_ = nullptr;
	PipelineState_Impl* impl = nullptr;
	std::array<Shader*, static_cast<int>(ShaderStageType::Max)> shaders;

public:
	PipelineStateMetal();
	virtual ~PipelineStateMetal();

	bool Initialize(GraphicsMetal* graphics);
	void SetShader(ShaderStageType stage, Shader* shader) override;
	void Compile() override;

	std::array<Shader*, static_cast<int>(ShaderStageType::Max)> GetShaders() const { return shaders; }

	PipelineState_Impl* GetImpl() { return impl; }
};

} // namespace LLGI
