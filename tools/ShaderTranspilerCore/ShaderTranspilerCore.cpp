#include "ShaderTranspilerCore.h"

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <iostream>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <spirv_reflect.hpp>

#include "ResourceLimits.h"

namespace LLGI
{

EShLanguage GetGlslangShaderStage(ShaderStageType type)
{
	if (type == ShaderStageType::Vertex)
		return EShLanguage::EShLangVertex;
	if (type == ShaderStageType::Pixel)
		return EShLanguage::EShLangFragment;
	throw std::string("Unimplemented ShaderStage");
}

SPIRV::SPIRV(const std::vector<uint32_t>& data, ShaderStageType shaderStage) : data_(data), shaderStage_(shaderStage) {}

SPIRV::SPIRV(const std::string& error) : error_(error) {}

ShaderStageType SPIRV::GetStage() const { return shaderStage_; }

const std::vector<uint32_t>& SPIRV::GetData() const { return data_; }

bool SPIRVTranspiler::Transpile(const std::shared_ptr<SPIRV>& spirv) { return false; }

std::string SPIRVTranspiler::GetErrorCode() const { return errorCode_; }

std::string SPIRVTranspiler::GetCode() const { return code_; }

bool SPIRVToHLSLTranspiler::Transpile(const std::shared_ptr<SPIRV>& spirv)
{
	spirv_cross::CompilerHLSL compiler(spirv->GetData());

	spirv_cross::CompilerGLSL::Options options;
	options.separate_shader_objects = true;
	compiler.set_common_options(options);

	spirv_cross::CompilerHLSL::Options targetOptions;
	compiler.set_hlsl_options(targetOptions);

	code_ = compiler.compile();

	return true;
}

bool SPIRVToMSLTranspiler::Transpile(const std::shared_ptr<SPIRV>& spirv)
{
	spirv_cross::CompilerMSL compiler(spirv->GetData());

	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	spirv_cross::CompilerGLSL::Options options;
	compiler.set_common_options(options);

	spirv_cross::CompilerMSL::Options targetOptions;
	compiler.set_msl_options(targetOptions);

	code_ = compiler.compile();

	return true;
}

bool SPIRVToGLSLTranspiler::Transpile(const std::shared_ptr<SPIRV>& spirv)
{
	spirv_cross::CompilerGLSL compiler(spirv->GetData());

	// to combine a sampler and a texture
	compiler.build_combined_image_samplers();

	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	int32_t binding_offset = 0;

	if (isVulkanMode_)
	{
		binding_offset += 1;
	}

	for (auto& resource : resources.sampled_images)
	{
		auto i = compiler.get_decoration(resource.id, spv::DecorationLocation);
		compiler.set_decoration(resource.id, spv::DecorationBinding, binding_offset + i);
	}

	for (auto& resource : resources.uniform_buffers)
	{
		if (spirv->GetStage() == ShaderStageType::Vertex)
		{
			if (isVulkanMode_)
			{
				compiler.set_decoration(resource.id, spv::DecorationBinding, 0);
				compiler.set_decoration(resource.id, spv::DecorationDescriptorSet, 0);
			}
		}
		else if (spirv->GetStage() == ShaderStageType::Pixel)
		{
			if (isVulkanMode_)
			{
				compiler.set_decoration(resource.id, spv::DecorationBinding, 0);
				compiler.set_decoration(resource.id, spv::DecorationDescriptorSet, 1);
			}
		}
	}

	spirv_cross::CompilerGLSL::Options options;
	options.version = 420;
	options.enable_420pack_extension = true;
	options.vulkan_semantics = isVulkanMode_;
	compiler.set_common_options(options);

	code_ = compiler.compile();

	return true;
}

SPIRVGenerator::SPIRVGenerator() { glslang::InitializeProcess(); }

SPIRVGenerator::~SPIRVGenerator() { glslang::FinalizeProcess(); }

std::shared_ptr<SPIRV> SPIRVGenerator::Generate(const char* code, ShaderStageType shaderStageType, bool isYInverted)
{
	std::string codeStr(code);
	glslang::TProgram program;
	TBuiltInResource resources = glslang::DefaultTBuiltInResource;
	auto shaderStage = GetGlslangShaderStage(shaderStageType);

	glslang::TShader shader = glslang::TShader(shaderStage);
	shader.setEnvInput(glslang::EShSourceHlsl, shaderStage, glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
	shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
	
	if (isYInverted)
	{
		shader.setInvertY(true);
	}

	const char* shaderStrings[1];
	shaderStrings[0] = codeStr.c_str();
	shader.setEntryPoint("main");
	// shader->setAutoMapBindings(true);
	// shader->setAutoMapLocations(true);

	shader.setStrings(shaderStrings, 1);
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	messages = (EShMessages)(messages | EShMsgReadHlsl);
	messages = (EShMessages)(messages | EShOptFull);

	int defaultVersion = 110;
	if (!shader.parse(&resources, defaultVersion, false, messages))
	{
		return std::make_shared<SPIRV>(shader.getInfoLog());
	}

	program.addShader(&shader);

	if (!program.link(messages))
	{
		return std::make_shared<SPIRV>(program.getInfoLog());
	}

	std::vector<unsigned int> spirv;
	glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirv);

	return std::make_shared<SPIRV>(spirv, shaderStageType);
}

} // namespace LLGI
