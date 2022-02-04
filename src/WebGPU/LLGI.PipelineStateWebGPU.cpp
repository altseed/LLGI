#include "LLGI.PipelineStateWebGPU.h"
#include "LLGI.ShaderWebGPU.h"
#include <limits>

namespace LLGI
{

PipelineStateWebGPU::PipelineStateWebGPU() { shaders_.fill(nullptr); }

PipelineStateWebGPU::~PipelineStateWebGPU()
{
	for (auto& shader : shaders_)
	{
		SafeRelease(shader);
	}
}

void PipelineStateWebGPU::SetShader(ShaderStageType stage, Shader* shader)
{
	SafeAddRef(shader);
	SafeRelease(shaders_[static_cast<int>(stage)]);
	shaders_[static_cast<int>(stage)] = shader;
}

void PipelineStateWebGPU::SetRenderPassPipelineState(RenderPassPipelineState* renderPassPipelineState) { throw "Unimpleneted"; }

bool PipelineStateWebGPU::Compile()
{
	wgpu::RenderPipelineDescriptor desc{};

	desc.primitive.topology = Convert(Topology);
	desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined; // is it correct?
	desc.primitive.frontFace = wgpu::FrontFace::CW;
	desc.primitive.cullMode = Convert(Culling);
	desc.multisample.count = 1;
	desc.multisample.mask = std::numeric_limits<int32_t>::max();
	desc.multisample.alphaToCoverageEnabled = false;
	desc.layout = nullptr; // is it correct?

	auto vertexShader = static_cast<ShaderWebGPU*>(shaders_[static_cast<int>(ShaderStageType::Vertex)]);
	const char* entryPointName = "main";

	desc.vertex.module = vertexShader->GetShaderModule();
	desc.vertex.entryPoint = entryPointName;

	desc.vertex.bufferCount = 1;
	std::array<wgpu::VertexBufferLayout, 1> bufferLayouts;
	desc.vertex.buffers = bufferLayouts.data();

	bufferLayouts[0].attributeCount = VertexLayoutCount;

	std::array<wgpu::VertexAttribute, VertexLayoutMax> attributes;
	bufferLayouts[0].attributes = attributes.data();

	int offset = 0;
	for (int i = 0; i < VertexLayoutCount; i++)
	{
		attributes[i].format = Convert(VertexLayouts[i]);
		attributes[i].offset = offset;
		attributes[i].shaderLocation = i;
		offset += GetSize(VertexLayouts[i]);
	}

	auto pixelShader = static_cast<ShaderWebGPU*>(shaders_[static_cast<int>(ShaderStageType::Pixel)]);

	wgpu::BlendState blendState;
	// blendState.color.srcFactor = Convert(BlendSrcFunc);

	throw "Unimpleneted";

	/*
		// setup layouts

		vk::PipelineVertexInputStateCreateInfo inputStateInfo;
		inputStateInfo.pVertexBindingDescriptions = bindDescs.data();
		inputStateInfo.pVertexAttributeDescriptions = attribDescs.data();
		inputStateInfo.vertexBindingDescriptionCount = static_cast<int32_t>(bindDescs.size());
		inputStateInfo.vertexAttributeDescriptionCount = static_cast<int32_t>(attribDescs.size());
		graphicsPipelineInfo.pVertexInputState = &inputStateInfo;
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
		scissor.offset = vk::Offset2D(0, 0);
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

		rasterizationState.depthBiasEnable = false;
		rasterizationState.depthBiasConstantFactor = 0.0f;
		rasterizationState.depthBiasClamp = 0.0f;
		rasterizationState.depthBiasSlopeFactor = 0.0f;
		rasterizationState.lineWidth = 1.0f; // disable lineWidth. (Must not be zero)

		graphicsPipelineInfo.pRasterizationState = &rasterizationState;

		assert(renderPassPipelineState_ != nullptr);
		auto renderPassPipelineState = static_cast<RenderPassPipelineStateVulkan*>(renderPassPipelineState_.get());
		auto renderPass = renderPassPipelineState->GetRenderPass();

		// setup a multisampling
		vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
		multisampleStateInfo.sampleShadingEnable = renderPassPipelineState->Key.SamplingCount > 1;
		multisampleStateInfo.rasterizationSamples = (vk::SampleCountFlagBits)renderPassPipelineState->Key.SamplingCount;
		multisampleStateInfo.minSampleShading = 1.0f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = false;
		multisampleStateInfo.alphaToOneEnable = false;

		graphicsPipelineInfo.pMultisampleState = &multisampleStateInfo;

		// setup a depthstencil
		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;

		// DepthTest flag must be enabled because DepthWrite and Stencil are depended on DepthTestFlag
		depthStencilInfo.depthTestEnable = true;

		depthStencilInfo.depthWriteEnable = IsDepthWriteEnabled;

		std::array<vk::CompareOp, 10> compareOps;
		compareOps[static_cast<int>(DepthFuncType::Never)] = vk::CompareOp::eNever;
		compareOps[static_cast<int>(DepthFuncType::Less)] = vk::CompareOp::eLess;
		compareOps[static_cast<int>(DepthFuncType::Equal)] = vk::CompareOp::eEqual;
		compareOps[static_cast<int>(DepthFuncType::LessEqual)] = vk::CompareOp::eLessOrEqual;
		compareOps[static_cast<int>(DepthFuncType::Greater)] = vk::CompareOp::eGreater;
		compareOps[static_cast<int>(DepthFuncType::NotEqual)] = vk::CompareOp::eNotEqual;
		compareOps[static_cast<int>(DepthFuncType::GreaterEqual)] = vk::CompareOp::eGreaterOrEqual;
		compareOps[static_cast<int>(DepthFuncType::Always)] = vk::CompareOp::eAlways;

		depthStencilInfo.depthCompareOp = compareOps[static_cast<int>(DepthFunc)];

		if (!IsDepthTestEnabled)
		{
			depthStencilInfo.depthCompareOp = vk::CompareOp::eAlways;
		}

		vk::StencilOpState stencil;
		depthStencilInfo.stencilTestEnable = true;

		std::array<vk::StencilOp, 8> stencilOps;
		stencilOps[static_cast<int>(StencilOperatorType::Keep)] = vk::StencilOp::eKeep;
		stencilOps[static_cast<int>(StencilOperatorType::Zero)] = vk::StencilOp::eZero;
		stencilOps[static_cast<int>(StencilOperatorType::Replace)] = vk::StencilOp::eReplace;
		stencilOps[static_cast<int>(StencilOperatorType::IncClamp)] = vk::StencilOp::eIncrementAndClamp;
		stencilOps[static_cast<int>(StencilOperatorType::DecClamp)] = vk::StencilOp::eDecrementAndClamp;
		stencilOps[static_cast<int>(StencilOperatorType::Invert)] = vk::StencilOp::eInvert;
		stencilOps[static_cast<int>(StencilOperatorType::IncRepeat)] = vk::StencilOp::eIncrementAndWrap;
		stencilOps[static_cast<int>(StencilOperatorType::DecRepeat)] = vk::StencilOp::eDecrementAndWrap;

		if (IsStencilTestEnabled)
		{
			stencil.depthFailOp = stencilOps[static_cast<int>(StencilDepthFailOp)];
			stencil.failOp = stencilOps[static_cast<int>(StencilFailOp)];
			stencil.passOp = stencilOps[static_cast<int>(StencilPassOp)];
			stencil.compareOp = compareOps[static_cast<int>(StencilCompareFunc)];
			stencil.writeMask = StencilWriteMask;
			stencil.compareMask = StencilReadMask;
			stencil.reference = StencilRef;
		}
		else
		{
			stencil.depthFailOp = vk::StencilOp::eKeep;
			stencil.failOp = vk::StencilOp::eKeep;
			stencil.passOp = vk::StencilOp::eReplace;
			stencil.compareOp = vk::CompareOp::eAlways;
			stencil.writeMask = 0xff;
			stencil.compareMask = 0xff;
			stencil.reference = 0xff;
		}

		depthStencilInfo.front = stencil;
		depthStencilInfo.back = stencil;

		depthStencilInfo.minDepthBounds = 0.0f;
		depthStencilInfo.maxDepthBounds = 1.0f;
		depthStencilInfo.depthBoundsTestEnable = false;

		graphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;

		// blending
		std::array<vk::PipelineColorBlendAttachmentState, RenderTargetMax> blendInfos;

		for (int32_t i = 0; i < renderPassPipelineState->RenderTargetCount; i++)
		{
			auto& blendInfo = blendInfos[i];

			blendInfo.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
									   vk::ColorComponentFlagBits::eA;

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
				blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcAlpha)] = vk::BlendFactor::eOneMinusSrcAlpha;
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
		}

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendInfo.attachmentCount = renderPassPipelineState->RenderTargetCount;
		colorBlendInfo.pAttachments = blendInfos.data();
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

		// setup a render pass

		graphicsPipelineInfo.renderPass = renderPass;

		// uniform layout info
		std::array<vk::DescriptorSetLayoutBinding, TextureSlotMax + 1> uboLayoutBindings;
		uboLayoutBindings[0].binding = 0;
		uboLayoutBindings[0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		uboLayoutBindings[0].descriptorCount = 1;
		uboLayoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		uboLayoutBindings[0].pImmutableSamplers = nullptr;

		for (size_t i = 1; i < uboLayoutBindings.size(); i++)
		{
			uboLayoutBindings[i].binding = static_cast<uint32_t>(i);
			uboLayoutBindings[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
			uboLayoutBindings[i].descriptorCount = 1;
			uboLayoutBindings[i].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
			uboLayoutBindings[i].pImmutableSamplers = nullptr;
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
		descriptorSetLayoutInfo.bindingCount = static_cast<int32_t>(uboLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = uboLayoutBindings.data();

		descriptorSetLayouts[0] = graphics_->GetDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);
		descriptorSetLayouts[1] = graphics_->GetDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);

		vk::PipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.setLayoutCount = 2;
		layoutInfo.pSetLayouts = descriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		pipelineLayout_ = graphics_->GetDevice().createPipelineLayout(layoutInfo);
		graphicsPipelineInfo.layout = pipelineLayout_;

	#if VK_HEADER_VERSION >= 136
		// setup a pipeline
		const auto pipeline = graphics_->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineInfo);
		if (pipeline.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Cannnot create graphicPipeline: " + std::to_string(static_cast<int>(pipeline.result)));
		}
		pipeline_ = pipeline.value;
	#else
		pipeline_ = graphics_->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineInfo);
	#endif

		return true;
	*/
	return false;
}

} // namespace LLGI