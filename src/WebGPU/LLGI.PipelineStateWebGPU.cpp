#include "LLGI.PipelineStateWebGPU.h"
#include "LLGI.RenderPassPipelineStateWebGPU.h"
#include "LLGI.ShaderWebGPU.h"
#include <limits>

namespace LLGI
{

PipelineStateWebGPU::PipelineStateWebGPU(wgpu::Device device) : device_(device) { shaders_.fill(nullptr); }

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

	// TODO : support blend enabled
	wgpu::BlendState blendState;
	blendState.color.srcFactor = Convert(BlendSrcFunc);
	blendState.color.dstFactor = Convert(BlendDstFunc);
	blendState.color.operation = Convert(BlendEquationRGB);
	blendState.color.srcFactor = Convert(BlendSrcFuncAlpha);
	blendState.color.dstFactor = Convert(BlendDstFuncAlpha);
	blendState.color.operation = Convert(BlendEquationAlpha);

	std::array<wgpu::ColorTargetState, RenderTargetMax> colorTargetStates;

	for (size_t i = 0; i < renderPassPipelineState_->Key.RenderTargetFormats.size(); i++)
	{
		colorTargetStates[i].blend = &blendState;
		colorTargetStates[i].format = ConvertFormat(renderPassPipelineState_->Key.RenderTargetFormats.at(i));
		colorTargetStates[i].writeMask = wgpu::ColorWriteMask::All;
	}

	wgpu::FragmentState fragmentState = {};
	fragmentState.targetCount = static_cast<uint32_t>(renderPassPipelineState_->Key.RenderTargetFormats.size());
	fragmentState.targets = colorTargetStates.data();
	fragmentState.entryPoint = entryPointName;
	fragmentState.module = pixelShader->GetShaderModule();

	desc.fragment = &fragmentState;

	wgpu::DepthStencilState depthStencilState = {};
	depthStencilState.depthWriteEnabled = IsDepthWriteEnabled;

	if (IsDepthTestEnabled)
	{
		depthStencilState.depthCompare = Convert(DepthFunc);
	}
	else
	{
		depthStencilState.depthCompare = wgpu::CompareFunction::Always;
	}

	if (IsStencilTestEnabled)
	{
		wgpu::StencilFaceState fs;

		fs.compare = Convert(StencilCompareFunc);
		fs.depthFailOp = Convert(StencilDepthFailOp);
		fs.failOp = Convert(StencilFailOp);
		fs.passOp = Convert(StencilPassOp);

		depthStencilState.stencilFront = fs;
		depthStencilState.stencilBack = fs;

		depthStencilState.stencilWriteMask = StencilWriteMask;
		depthStencilState.stencilReadMask = StencilReadMask;
	}
	else
	{
		wgpu::StencilFaceState fs;

		fs.depthFailOp = wgpu::StencilOperation::Keep;
		fs.failOp = wgpu::StencilOperation::Keep;
		fs.compare = wgpu::CompareFunction::Always;
		fs.passOp = wgpu::StencilOperation::Replace;

		depthStencilState.stencilFront = fs;
		depthStencilState.stencilBack = fs;

		depthStencilState.stencilWriteMask = 0xff;
		depthStencilState.stencilReadMask = 0xff;
	}

	desc.depthStencil = &depthStencilState;

	renderPipeline_ = device_.CreateRenderPipeline(&desc);

	return renderPipeline_ != nullptr;
}

} // namespace LLGI