
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class VertexBuffer;
class IndexBuffer;

class CommandList
	: public ReferenceObject
{
private:
public:
	CommandList() = default;
	virtual ~CommandList() = default;

	virtual void Begin();
	virtual void End();
	virtual void Clear();
	virtual void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
	virtual void Draw();
	virtual void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride);
	virtual void SetIndexBuffer(IndexBuffer* indexBuffer);
	virtual void SetPipelineState(PipelineState* pipelineState);
};

}
}