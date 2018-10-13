
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class PipelineState
	: public ReferenceObject
{
private:
public:
	PipelineState() = default;
	virtual ~PipelineState() = default;

	CullingMode Culling = CullingMode::Clockwise;
	TopologyType Topology = TopologyType::Triangle;

	bool IsBlendEnabled = true;

	BlendFuncType BlendSrcFunc = BlendFuncType::SrcAlpha;
	BlendFuncType BlendDstFunc = BlendFuncType::OneMinusSrcAlpha;
	BlendFuncType BlendSrcFuncAlpha = BlendFuncType::SrcAlpha;
	BlendFuncType BlendDstFuncAlpha = BlendFuncType::OneMinusSrcAlpha;

	BlendEquationType BlendEquationRGB = BlendEquationType::Add;
	BlendEquationType BlendEquationAlpha = BlendEquationType::Add;

	bool IsDepthTestEnabled = false;
	bool IsDepthWriteEnabled = false;
	bool IsStencilTestEnabled = false;

	std::array<VertexLayoutFormat, 16> VertexLayouts;
	int32_t VertexLayoutCount = 0;

	virtual void SetShader(ShaderStageType stage, Shader* shader);
	virtual void Compile();
};

}
}