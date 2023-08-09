#include "LLGI.ShaderWebGPU.h"

namespace LLGI
{
ShaderWebGPU::ShaderWebGPU() {}

ShaderWebGPU::~ShaderWebGPU() {}

bool ShaderWebGPU::Initialize(wgpu::Device& device, DataStructure* data, int32_t count)
{
	if (data == nullptr || count == 0)
	{
		return false;
	}

	wgpu::ShaderModuleDescriptor desc;

	if (data[0].Size < 8)
	{
		return false;
	}

	// check whether binary or code
	bool isCode = false;
	const char* code = static_cast<const char*>(data[0].Data);

	if (code[0] == 'w' || code[1] == 'g' || code[2] == 's' || code[3] == 'l' || code[4] == 'c' || code[5] == 'o' || code[6] == 'd' ||
		code[7] == 'e')
	{
		isCode = true;
	}

	wgpu::ShaderModuleSPIRVDescriptor sprivDesc;
	wgpu::ShaderModuleWGSLDescriptor wgslDesc;

	if (isCode)
	{
		wgslDesc.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
		wgslDesc.code = reinterpret_cast<const char*>(data[0].Data);
		desc.nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&wgslDesc);
	}
	else
	{
		sprivDesc.sType = wgpu::SType::ShaderModuleSPIRVDescriptor;
		sprivDesc.codeSize = data[0].Size;
		sprivDesc.code = reinterpret_cast<const uint32_t*>(data[0].Data);
		desc.nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&sprivDesc);
	}

	shaderModule_ = device.CreateShaderModule(&desc);

	return shaderModule_ != nullptr;
}

wgpu::ShaderModule& ShaderWebGPU::GetShaderModule() { return shaderModule_; }

} // namespace LLGI
