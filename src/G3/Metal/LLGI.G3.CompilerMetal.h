
#pragma once

#include "../LLGI.G3.Compiler.h"

namespace LLGI
{
namespace G3
{

class CompilerMetal : public Compiler
{
private:
public:
	void Initialize() override;
	void Compile(CompilerResult& result, const char* code, ShaderStageType shaderStage) override;

	DeviceType GetDeviceType() const override { return DeviceType::Metal; }
};

} // namespace G3
} // namespace LLGI
