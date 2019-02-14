#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

Compiler* CreateCompiler(DeviceType device);

struct CompilerResult
{
	std::string Message;
	std::vector<std::vector<uint8_t>> Binary;
};

class Compiler : public ReferenceObject
{
private:
public:
	Compiler() = default;
	virtual ~Compiler() = default;

	virtual void Initialize();
	virtual void Compile(CompilerResult& result, const char* code, ShaderStageType shaderStage);

	virtual DeviceType GetDeviceType() const { return DeviceType::Default; }
};

} // namespace G3
} // namespace LLGI