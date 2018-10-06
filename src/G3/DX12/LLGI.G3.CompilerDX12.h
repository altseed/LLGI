
#pragma once

#include "../LLGI.G3.Compiler.h"
#include "LLGI.G3.BaseDX12.h"

namespace LLGI
{
namespace G3
{

class CompilerDX12
	: public Compiler
{

private:
public:
	CompilerDX12() = default;
	virtual ~CompilerDX12() = default;

	void Initialize() override;
	void Compile(CompilerResult& result, const char* code, ShaderStageType shaderStage) override;

	DeviceType GetDeviceType() const override { return DeviceType::DirectX12; }
};

}
}

