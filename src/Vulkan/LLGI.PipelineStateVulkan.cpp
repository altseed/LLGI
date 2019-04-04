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

	if (pipelineLayout != nullptr)
	{
		graphics_->GetDevice().destroyPipelineLayout(pipelineLayout);
		pipelineLayout = nullptr;
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

	// setup layouts
	std::vector<vk::VertexInputBindingDescription> bindDescs;
	std::vector<vk::VertexInputAttributeDescription> attribDescs;

	int vertexOffset = 0;
	for (int i = 0; i < VertexLayoutCount; i++)
	{
		vk::VertexInputAttributeDescription attribDesc;

		attribDesc.binding = 0;
		attribDesc.location = i;
		attribDesc.offset = vertexOffset;

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32B32_FLOAT)
		{
			attribDesc.format = vk::Format::eR32G32B32Sfloat;
			vertexOffset += sizeof(float) * 3;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32_FLOAT)
		{
			attribDesc.format = vk::Format::eR32G32Sfloat;
			vertexOffset += sizeof(float) * 2;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UINT)
		{
			attribDesc.format = vk::Format::eR8G8B8A8Uint;
			vertexOffset += sizeof(float);
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UNORM)
		{
			attribDesc.format = vk::Format::eR8G8B8A8Unorm;
			vertexOffset += sizeof(float);
		}

		attribDescs.push_back(attribDesc);
	}

	vk::VertexInputBindingDescription bindDesc;
	bindDesc.binding = 0;
	bindDesc.stride = vertexOffset;
	bindDesc.inputRate = vk::VertexInputRate::eVertex;
	bindDescs.push_back(bindDesc);

	vk::PipelineVertexInputStateCreateInfo inputStateInfo;
	inputStateInfo.pVertexBindingDescriptions = bindDescs.data();
	inputStateInfo.pVertexAttributeDescriptions = attribDescs.data();
	inputStateInfo.vertexBindingDescriptionCount = bindDescs.size();
	inputStateInfo.vertexAttributeDescriptionCount = attribDescs.size();
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

	// setup a viewport
	vk::PipelineViewportStateCreateInfo viewportStateInfo;

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)1280; // TODO : temp
	viewport.height = (float)720; // TODO : temp
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.viewportCount = 1;

	vk::Rect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent.width = 1280; // TODO : temp
	scissor.extent.height = 720; // TODO : temp

	viewportStateInfo.pScissors = &scissor;
	viewportStateInfo.scissorCount = 1;

	graphicsPipelineInfo.pViewportState = &viewportStateInfo;

	// setup a rasterlizer
	vk::PipelineRasterizationStateCreateInfo rasterizationState;
	rasterizationState.depthClampEnable = false;
	rasterizationState.rasterizerDiscardEnable = false;
	rasterizationState.polygonMode = vk::PolygonMode::eFill;

	if (Culling == CullingMode::Clockwise)
	{
		rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
	}
	else if (Culling == CullingMode::CounterClockwise)
	{
		rasterizationState.cullMode = vk::CullModeFlagBits::eFront;
	}
	else if (Culling == CullingMode::DoubleSide)
	{
		rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
	}

	rasterizationState.frontFace = vk::FrontFace::eClockwise;

	rasterizationState.depthBiasEnable = false;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp = 0.0f;
	rasterizationState.depthBiasSlopeFactor = 0.0f;

	graphicsPipelineInfo.pRasterizationState = &rasterizationState;

	// setup a multisampling
	vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
	multisampleStateInfo.sampleShadingEnable = false;
	multisampleStateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampleStateInfo.minSampleShading = 1.0f;
	multisampleStateInfo.pSampleMask = nullptr;
	multisampleStateInfo.alphaToCoverageEnable = false;
	multisampleStateInfo.alphaToOneEnable = false;

	graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;

	// setup a depthstencil
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;

	depthStencilInfo.depthTestEnable = IsDepthTestEnabled;
	depthStencilInfo.depthWriteEnable = IsDepthWriteEnabled;
	depthStencilInfo.stencilTestEnable = IsStencilTestEnabled;

	std::array<vk::CompareOp, 10> depthCompareOps;
	depthCompareOps[static_cast<int>(DepthFuncType::Never)] = vk::CompareOp::eNever;
	depthCompareOps[static_cast<int>(DepthFuncType::Less)] = vk::CompareOp::eLess;
	depthCompareOps[static_cast<int>(DepthFuncType::Equal)] = vk::CompareOp::eEqual;
	depthCompareOps[static_cast<int>(DepthFuncType::LessEqual)] = vk::CompareOp::eLessOrEqual;
	depthCompareOps[static_cast<int>(DepthFuncType::Greater)] = vk::CompareOp::eGreater;
	depthCompareOps[static_cast<int>(DepthFuncType::NotEqual)] = vk::CompareOp::eNotEqual;
	depthCompareOps[static_cast<int>(DepthFuncType::GreaterEqual)] = vk::CompareOp::eGreaterOrEqual;
	depthCompareOps[static_cast<int>(DepthFuncType::Always)] = vk::CompareOp::eAlways;

	depthStencilInfo.depthCompareOp = depthCompareOps[static_cast<int>(DepthFunc)];

	graphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;

	// blending
	vk::PipelineColorBlendAttachmentState blendInfo;
	blendInfo.colorWriteMask =
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	if (IsBlendEnabled)
	{
		blendInfo.blendEnable = true;

		std::array<vk::BlendOp, 10> blendOps;
		blendOps[static_cast<int>(BlendEquationType::Add)] = vk::BlendOp::eAdd;
		blendOps[static_cast<int>(BlendEquationType::Sub)] = vk::BlendOp::eSubtract;
		blendOps[static_cast<int>(BlendEquationType::ReverseSub)] = vk::BlendOp::eReverseSubtract;
		blendOps[static_cast<int>(BlendEquationType::Min)] = vk::BlendOp::eMin;
		blendOps[static_cast<int>(BlendEquationType::Max)] = vk::BlendOp::eMax;

		std::array<vk::BlendFactor, 20> blendFuncs;
		blendFuncs[static_cast<int>(BlendFuncType::Zero)] = vk::BlendFactor::eZero;
		blendFuncs[static_cast<int>(BlendFuncType::One)] = vk::BlendFactor::eOne;
		blendFuncs[static_cast<int>(BlendFuncType::SrcColor)] = vk::BlendFactor::eSrcColor;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcColor)] = vk::BlendFactor::eOneMinusSrcColor;
		blendFuncs[static_cast<int>(BlendFuncType::SrcAlpha)] = vk::BlendFactor::eSrcAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcAlpha)] = vk::BlendFactor::eSrcAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::DstColor)] = vk::BlendFactor::eDstColor;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstColor)] = vk::BlendFactor::eOneMinusDstColor;
		blendFuncs[static_cast<int>(BlendFuncType::DstAlpha)] = vk::BlendFactor::eDstAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstAlpha)] = vk::BlendFactor::eDstAlpha;

		blendInfo.srcColorBlendFactor = blendFuncs[static_cast<int>(BlendSrcFunc)];
		blendInfo.dstColorBlendFactor = blendFuncs[static_cast<int>(BlendDstFunc)];
		blendInfo.srcAlphaBlendFactor = blendFuncs[static_cast<int>(BlendSrcFuncAlpha)];
		blendInfo.dstAlphaBlendFactor = blendFuncs[static_cast<int>(BlendDstFuncAlpha)];
		blendInfo.colorBlendOp = blendOps[static_cast<int>(BlendEquationRGB)];
		blendInfo.alphaBlendOp = blendOps[static_cast<int>(BlendEquationAlpha)];
	}
	else
	{
		blendInfo.blendEnable = false;
	}

	vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &blendInfo;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;

	graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;

	// dynamic state
	vk::DynamicState dynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
	dynamicStateInfo.pDynamicStates = dynamicStates;
	dynamicStateInfo.dynamicStateCount = 2;

	graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;

	// render pass TODO
	// graphicsPipelineInfo.renderPass
	assert(0);

	// pipeline layout
	vk::PipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.setLayoutCount = 0;
	layoutInfo.pSetLayouts = nullptr;
	layoutInfo.pushConstantRangeCount = 0;
	layoutInfo.pPushConstantRanges = nullptr;

	pipelineLayout = graphics_->GetDevice().createPipelineLayout(layoutInfo);

	// setup a pipeline
	pipeline = graphics_->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineInfo);
}

} // namespace LLGI