
#include "LLGI.ShaderDX12.h"
#include "../LLGI.Shader.h"
#include "LLGI.CompilerDX12.h"

namespace LLGI
{

bool ShaderDX12::Initialize(DataStructure* data, int32_t count)
{
	for (int i = 0; i < count; i++)
		data_.push_back(data[i]);
	count_ = count;
	return true;
}

} // namespace LLGI
