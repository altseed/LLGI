#include "LLGI.G3.Graphics.h"

namespace LLGI
{
namespace G3
{

void Graphics::Execute(CommandList* commandList)
{

}

VertexBuffer* Graphics::CreateVertexBuffer(int32_t size)
{
	return nullptr;
}

IndexBuffer* Graphics::CreateIndexBuffer(int32_t stride, int32_t count)
{
	return nullptr;
}

Shader* Graphics::CreateShader(DataStructure* data, int32_t count)
{
	return nullptr;
}

PipelineState* Graphics::CreatePiplineState()
{
	return nullptr;
}

CommandList* Graphics::CreateCommandList()
{
	return nullptr;
}

}
}