#include "LLGI.PipelineStateVulkan.h"
#include "LLGI.ShaderVulkan.h"

namespace LLGI
{

PipelineStateVulkan::PipelineStateVulkan() {}

PipelineStateVulkan ::~PipelineStateVulkan()
{
	for (auto& shader : shaders)
	{
		SafeRelease(shader);
	}

	if (pipeline != nullptr)
	{
		graphics_->GetDevice().destroyPipeline(pipeline);
		pipeline = nullptr;
	}

	SafeRelease(graphics_);
}

bool PipelineStateVulkan::Initialize(GraphicsVulkan* graphics)
{
	SafeRelease(graphics_);
	SafeAddRef(graphics);
	graphics_ = graphics;
	return true;
}

void PipelineStateVulkan::SetShader(ShaderStageType stage, Shader* shader)
{

	SafeAddRef(shader);
	SafeRelease(shaders[static_cast<int>(stage)]);
	shaders[static_cast<int>(stage)] = shader;
}

void PipelineStateVulkan::Compile()
{
	vk::GraphicsPipelineCreateInfo graphicsPipelineInfo;

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

	// setup shaders
	std::string mainName = "main";

	for (size_t i = 0; i < this->shaders.size(); i++)
	{
		auto shader = static_cast<ShaderVulkan*>(shaders[i]);

		vk::PipelineShaderStageCreateInfo info;

		if (i == static_cast<int>(ShaderStageType::Vertex))
			info.stage = vk::ShaderStageFlagBits::eVertex;

		if (i == static_cast<int>(ShaderStageType::Pixel))
			info.stage = vk::ShaderStageFlagBits::eFragment;

		info.module = shader->GetShaderModule();
		info.pName = mainName.c_str();
		shaderStageInfos.push_back(info);
	}

	graphicsPipelineInfo.pStages = shaderStageInfos.data();
	graphicsPipelineInfo.stageCount = shaderStageInfos.size();

	// setup layouts (this is empty because these parameters are written in shaders)
	vk::PipelineVertexInputStateCreateInfo inputStateInfo;
	inputStateInfo.pVertexBindingDescriptions = nullptr;
	inputStateInfo.pVertexAttributeDescriptions = nullptr;
	inputStateInfo.vertexAttributeDescriptionCount = 0;
	inputStateInfo.vertexAttributeDescriptionCount = 0;
	graphicsPipelineInfo.pVertexInputState = &inputStateInfo;

	// setup a topology
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
	if (Topology == TopologyType::Triangle)
	{
		inputAssemblyStateInfo.topology = vk::PrimitiveTopology::eTriangleList;
	}
	else if (Topology == TopologyType::Line)
	{
		inputAssemblyStateInfo.topology = vk::PrimitiveTopology::eLineList;
	}
	else
	{
		assert(0);
	}
	inputAssemblyStateInfo.primitiveRestartEnable = false;
	
	graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
	
	// TODO Viewport

	// TODO rasterlizer

	// TODO multisampling

	// TODO blending
	
	pipeline = graphics_->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineInfo);
}

} // namespace LLGI