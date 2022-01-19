#pragma once

#include "LLGI.BaseWebGPU.h"
#include "../LLGI.Shader.h"

namespace LLGI
{

class ShaderWebGPU : public Shader
{
private:
    wgpu::ShaderModule shaderModule_;

public:
	ShaderWebGPU();
	~ShaderWebGPU() override;

    bool Initialize(wgpu::Device& device, DataStructure* data, int32_t count);

    wgpu::ShaderModule& GetShaderModule();
};

}