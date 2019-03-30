
#pragma once

#include "../LLGI.Compiler.h"
#include "LLGI.BaseDX12.h"

namespace LLGI
{

class CompilerDX12 : public Compiler
{

private:
public:
	CompilerDX12() = default;
	virtual ~CompilerDX12() = default;

	void Initialize() override;
	void Compile(CompilerResult& result, const char* code, ShaderStageType shaderStage) override;

	DeviceType GetDeviceType() const override { return DeviceType::DirectX12; }
};

} // namespace LLGI
