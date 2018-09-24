
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class Graphics
	: public ReferenceObject
{
private:
public:
	Graphics() = default;
	virtual ~Graphics() = default;

	virtual void Execute(CommandList* commandList);

	virtual VertexBuffer* CreateVertexBuffer(int32_t size);
	virtual IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count);
	virtual Shader* CreateShader(DataStructure* data, int32_t count);
	virtual PipelineState* CreatePiplineState();
	virtual CommandList* CreateCommandList();
};

}
}