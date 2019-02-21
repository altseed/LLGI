
#include "LLGI.G3.ShaderDX12.h"
#include "../LLGI.G3.Shader.h"
#include "LLGI.G3.CompilerDX12.h"

namespace LLGI
{
namespace G3
{

bool ShaderDX12::Initialize(DataStructure* data, int32_t count)
{
	for (int i = 0; i < count; i++)
		data_.push_back(data[i]);
	count_ = count;
	return true;
}

} // namespace G3
} // namespace LLGI
