
#pragma once

#include "../LLGI.G3.Shader.h"
#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

using namespace DirectX;

namespace LLGI
{
namespace G3
{

class ShaderDX12 : public Shader
{
private:
	std::vector<DataStructure> data_;
	int32_t count_;

public:
	ShaderDX12() = default;
	virtual ~ShaderDX12() = default;

	bool Initialize(DataStructure* data, int32_t count);

	DataStructure* GetData() { return data_.data(); }
	int32_t GetCount() { return count_; }
};

} // namespace G3
} // namespace LLGI
