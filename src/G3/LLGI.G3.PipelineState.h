
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class PipelineState
{
private:
public:
	PipelineState() = default;
	virtual ~PipelineState() = default;

	CullingMode Culling = CullingMode::Clockwise;

	std::array<VertexLayoutFormat, 16> VertexLayouts;
	int32_t VertexLayoutCount = 0;

	virtual void SetShader(ShaderStageType stage, Shader* shader);
	virtual void Compile();
};

}
}