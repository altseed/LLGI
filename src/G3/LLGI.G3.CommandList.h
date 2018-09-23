
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class VertexBuffer;
class IndexBuffer;

class CommandList
{
private:
public:

	virtual void Begin();
	virtual void End();
	virtual void Clear();
	virtual void Draw();
	virtual void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride);
	virtual void SetIndexBuffer(IndexBuffer* indexBuffer);
	virtual void SetPipelineState(PipelineState* pipelineState);
};

}
}