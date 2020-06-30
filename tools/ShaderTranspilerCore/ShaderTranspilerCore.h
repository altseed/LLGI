
#pragma once

#include <stdint.h>

#include "../../src/LLGI.Base.h"
#include <memory>
#include <string>
#include <vector>

namespace LLGI
{
class SPIRV
{
private:
	std::vector<uint32_t> data_;
	std::string error_;
	ShaderStageType shaderStage_;

public:
	SPIRV(const std::vector<uint32_t>& data, ShaderStageType shaderStage);

	SPIRV(const std::string& error);

	ShaderStageType GetStage() const;

	const std::vector<uint32_t>& GetData() const;

	std::string GetError() const { return error_; }
};

class SPIRVTranspiler
{
protected:
	std::string code_;
	std::string errorCode_;

public:
	SPIRVTranspiler() = default;
	virtual ~SPIRVTranspiler() = default;

	virtual bool Transpile(const std::shared_ptr<SPIRV>& spirv);
	std::string GetErrorCode() const;
	std::string GetCode() const;
};

/**
	@brief it doesn't work currently
*/

class SPIRVToHLSLTranspiler : public SPIRVTranspiler
{
public:
	bool Transpile(const std::shared_ptr<SPIRV>& spirv) override;
};

class SPIRVToMSLTranspiler : public SPIRVTranspiler
{
public:
	bool Transpile(const std::shared_ptr<SPIRV>& spirv) override;
};

class SPIRVToGLSLTranspiler : public SPIRVTranspiler
{
private:
	bool isVulkanMode_ = false;

public:
	SPIRVToGLSLTranspiler(bool isVulkanMode) : isVulkanMode_(isVulkanMode) {}

	bool Transpile(const std::shared_ptr<SPIRV>& spirv) override;
};

class SPIRVGenerator
{
private:
public:
	SPIRVGenerator();

	~SPIRVGenerator();

	std::shared_ptr<SPIRV> Generate(const char* code, ShaderStageType shaderStageType, bool isYInverted);
};

} // namespace LLGI
