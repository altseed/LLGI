#include "LLGI.PipelineStateMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.RenderPassMetal.h"
#include "LLGI.ShaderMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

bool PipelineStateMetal::Compile(PipelineState* self, Graphics* graphics)
{
    auto g = static_cast<GraphicsMetal*>(graphics);

	auto self_ = static_cast<PipelineStateMetal*>(self);
	auto renderPassPipelineStateMetal_ = static_cast<RenderPassPipelineStateMetal*>(self_->GetRenderPassPipelineState());

	pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

	// vertex layout
	MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];

	int vertexOffset = 0;
	for (int i = 0; i < self_->VertexLayoutCount; i++)
	{
		vertexDescriptor.attributes[i].offset = vertexOffset;

		if (self_->VertexLayouts[i] == VertexLayoutFormat::R32G32B32_FLOAT)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float) * 3;
		}
		else if (self_->VertexLayouts[i] == VertexLayoutFormat::R32G32B32A32_FLOAT)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float) * 4;
		}
		else if (self_->VertexLayouts[i] == VertexLayoutFormat::R32G32_FLOAT)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float) * 2;
		}
		else if (self_->VertexLayouts[i] == VertexLayoutFormat::R32_FLOAT)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float) * 1;
		}
		else if (self_->VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UINT)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatUChar4;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float);
		}
		else if (self_->VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UNORM)
		{
			vertexDescriptor.attributes[i].format = MTLVertexFormatUChar4Normalized;
			vertexDescriptor.attributes[i].bufferIndex = VertexBufferIndex;
			vertexOffset += sizeof(float);
		}
		else
		{
			Log(LogType::Error, "Unimplemented VertexLoayoutFormat");
			return false;
		}
	}

	vertexDescriptor.layouts[VertexBufferIndex].stepRate = 1;
	vertexDescriptor.layouts[VertexBufferIndex].stepFunction = MTLVertexStepFunctionPerVertex;
	vertexDescriptor.layouts[VertexBufferIndex].stride = vertexOffset;

	pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;

	// setup shaders
	auto vs = static_cast<ShaderMetal*>(self_->GetShaders()[static_cast<int>(ShaderStageType::Vertex)]);
	auto ps = static_cast<ShaderMetal*>(self_->GetShaders()[static_cast<int>(ShaderStageType::Pixel)]);

	id<MTLFunction> vf = [vs->GetLibrary() newFunctionWithName:@"main0"];
	id<MTLFunction> pf = [ps->GetLibrary() newFunctionWithName:@"main0"];
	pipelineStateDescriptor.vertexFunction = vf;
	pipelineStateDescriptor.fragmentFunction = pf;

	// setup a depth
	if (renderPassPipelineStateMetal_->GetDepthStencilFormat() != MTLPixelFormatInvalid)
	{

		MTLDepthStencilDescriptor* depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
		depthStencilDescriptor.depthWriteEnabled = self_->IsDepthWriteEnabled;

		if (self_->IsDepthTestEnabled)
		{
			std::array<MTLCompareFunction, 10> depthCompareOps;
			depthCompareOps[static_cast<int>(DepthFuncType::Never)] = MTLCompareFunctionNever;
			depthCompareOps[static_cast<int>(DepthFuncType::Less)] = MTLCompareFunctionLess;
			depthCompareOps[static_cast<int>(DepthFuncType::Equal)] = MTLCompareFunctionEqual;
			depthCompareOps[static_cast<int>(DepthFuncType::LessEqual)] = MTLCompareFunctionLessEqual;
			depthCompareOps[static_cast<int>(DepthFuncType::Greater)] = MTLCompareFunctionGreater;
			depthCompareOps[static_cast<int>(DepthFuncType::NotEqual)] = MTLCompareFunctionNotEqual;
			depthCompareOps[static_cast<int>(DepthFuncType::GreaterEqual)] = MTLCompareFunctionGreaterEqual;
			depthCompareOps[static_cast<int>(DepthFuncType::Always)] = MTLCompareFunctionAlways;
			depthStencilDescriptor.depthCompareFunction = depthCompareOps[static_cast<int>(self_->DepthFunc)];
		}
		else
		{
			depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
		}

		if (renderPassPipelineStateMetal_->Key.DepthFormat == TextureFormatType::D24S8 ||
			renderPassPipelineStateMetal_->Key.DepthFormat == TextureFormatType::D32S8)
		{
			MTLStencilDescriptor* stencilDescriptor = [[MTLStencilDescriptor alloc] init];

			if (self_->IsStencilTestEnabled)
			{
				std::array<MTLStencilOperation, 8> stencilOps;
				stencilOps[static_cast<int>(StencilOperatorType::Keep)] = MTLStencilOperationKeep;
				stencilOps[static_cast<int>(StencilOperatorType::Zero)] = MTLStencilOperationZero;
				stencilOps[static_cast<int>(StencilOperatorType::Replace)] = MTLStencilOperationReplace;
				stencilOps[static_cast<int>(StencilOperatorType::IncClamp)] = MTLStencilOperationIncrementClamp;
				stencilOps[static_cast<int>(StencilOperatorType::DecClamp)] = MTLStencilOperationDecrementClamp;
				stencilOps[static_cast<int>(StencilOperatorType::Invert)] = MTLStencilOperationDecrementClamp;
				stencilOps[static_cast<int>(StencilOperatorType::IncRepeat)] = MTLStencilOperationIncrementWrap;
				stencilOps[static_cast<int>(StencilOperatorType::DecRepeat)] = MTLStencilOperationDecrementWrap;

				std::array<MTLCompareFunction, 8> stencilCompareFuncs;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::Never)] = MTLCompareFunctionNever;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::Less)] = MTLCompareFunctionLess;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::Equal)] = MTLCompareFunctionEqual;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::LessEqual)] = MTLCompareFunctionLessEqual;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::Greater)] = MTLCompareFunctionGreater;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::NotEqual)] = MTLCompareFunctionNotEqual;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::GreaterEqual)] = MTLCompareFunctionGreaterEqual;
				stencilCompareFuncs[static_cast<int>(CompareFuncType::Always)] = MTLCompareFunctionAlways;

				stencilDescriptor.depthFailureOperation = stencilOps[static_cast<int>(self_->StencilDepthFailOp)];
				stencilDescriptor.stencilFailureOperation = stencilOps[static_cast<int>(self_->StencilFailOp)];
				stencilDescriptor.depthStencilPassOperation = stencilOps[static_cast<int>(self_->StencilPassOp)];
				stencilDescriptor.stencilCompareFunction = stencilCompareFuncs[static_cast<int>(self_->StencilCompareFunc)];
				stencilDescriptor.readMask = self_->StencilReadMask;
				stencilDescriptor.writeMask = self_->StencilWriteMask;
			}
			else
			{
				// always write to stencil reference value
				stencilDescriptor.depthFailureOperation = MTLStencilOperationKeep;
				stencilDescriptor.stencilFailureOperation = MTLStencilOperationKeep;
				stencilDescriptor.depthStencilPassOperation = MTLStencilOperationReplace;
				stencilDescriptor.stencilCompareFunction = MTLCompareFunctionAlways;
				stencilDescriptor.readMask = 0xFF;
				stencilDescriptor.writeMask = 0xFF;
			}

			depthStencilDescriptor.frontFaceStencil = stencilDescriptor;
			depthStencilDescriptor.backFaceStencil = stencilDescriptor;
			[stencilDescriptor release];
		}

		depthStencilState = [g->GetDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];
		[depthStencilDescriptor release];
	}

	// topology
	if (self_->Topology == TopologyType::Triangle)
	{
		if (@available(iOS 12.0, *))
		{
			pipelineStateDescriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
		}
		else
		{
			// Fallback on earlier versions
		}
	}
	else if (self_->Topology == TopologyType::Line)
	{
		if (@available(iOS 12.0, *))
		{
			pipelineStateDescriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassLine;
		}
		else
		{
			Log(LogType::Error, "OS is too old.");
			return false;
		}
	}
	else if (self_->Topology == TopologyType::Point)
	{
		if (@available(iOS 12.0, *))
		{
			pipelineStateDescriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;
		}
		else
		{
			Log(LogType::Error, "OS is too old.");
			return false;
		}
	}
	else
	{
		Log(LogType::Error, "Unimplemented TopologyType");
		return false;
	}

	// clulling (on commandlist)

	// setup a blend

	MTLRenderPipelineColorAttachmentDescriptor* colorAttachment = pipelineStateDescriptor.colorAttachments[0];

	if (self_->IsBlendEnabled)
	{
		colorAttachment.blendingEnabled = true;

		std::array<MTLBlendOperation, 10> blendOps;
		blendOps[static_cast<int>(BlendEquationType::Add)] = MTLBlendOperationAdd;
		blendOps[static_cast<int>(BlendEquationType::Sub)] = MTLBlendOperationSubtract;
		blendOps[static_cast<int>(BlendEquationType::ReverseSub)] = MTLBlendOperationReverseSubtract;
		blendOps[static_cast<int>(BlendEquationType::Min)] = MTLBlendOperationMin;
		blendOps[static_cast<int>(BlendEquationType::Max)] = MTLBlendOperationMax;

		std::array<MTLBlendFactor, 20> blendFuncs;
		blendFuncs[static_cast<int>(BlendFuncType::Zero)] = MTLBlendFactorZero;
		blendFuncs[static_cast<int>(BlendFuncType::One)] = MTLBlendFactorOne;
		blendFuncs[static_cast<int>(BlendFuncType::SrcColor)] = MTLBlendFactorSourceColor;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcColor)] = MTLBlendFactorOneMinusSourceColor;
		blendFuncs[static_cast<int>(BlendFuncType::SrcAlpha)] = MTLBlendFactorSourceAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcAlpha)] = MTLBlendFactorOneMinusSourceAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::DstColor)] = MTLBlendFactorDestinationColor;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstColor)] = MTLBlendFactorOneMinusDestinationColor;
		blendFuncs[static_cast<int>(BlendFuncType::DstAlpha)] = MTLBlendFactorDestinationAlpha;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstAlpha)] = MTLBlendFactorOneMinusDestinationAlpha;

		colorAttachment.sourceRGBBlendFactor = blendFuncs[static_cast<int>(self_->BlendSrcFunc)];
		colorAttachment.destinationRGBBlendFactor = blendFuncs[static_cast<int>(self_->BlendDstFunc)];
		colorAttachment.sourceAlphaBlendFactor = blendFuncs[static_cast<int>(self_->BlendSrcFuncAlpha)];
		colorAttachment.destinationAlphaBlendFactor = blendFuncs[static_cast<int>(self_->BlendDstFuncAlpha)];
		colorAttachment.rgbBlendOperation = blendOps[static_cast<int>(self_->BlendEquationRGB)];
		colorAttachment.alphaBlendOperation = blendOps[static_cast<int>(self_->BlendEquationAlpha)];
	}
	else
	{
		colorAttachment.blendingEnabled = false;
	}

	for (size_t i = 0; i < renderPassPipelineStateMetal_->GetPixelFormats().size(); i++)
	{
		[pipelineStateDescriptor.colorAttachments objectAtIndexedSubscript:i].pixelFormat =
			renderPassPipelineStateMetal_->GetPixelFormats().at(i);
	}

	if (renderPassPipelineStateMetal_->GetDepthStencilFormat() != MTLPixelFormatInvalid)
	{
		pipelineStateDescriptor.depthAttachmentPixelFormat = renderPassPipelineStateMetal_->GetDepthStencilFormat();

		if (renderPassPipelineStateMetal_->Key.DepthFormat == TextureFormatType::D24S8 ||
			renderPassPipelineStateMetal_->Key.DepthFormat == TextureFormatType::D32S8)
		{
			pipelineStateDescriptor.stencilAttachmentPixelFormat = renderPassPipelineStateMetal_->GetDepthStencilFormat();
		}
	}

	pipelineStateDescriptor.sampleCount = renderPassPipelineStateMetal_->Key.SamplingCount;

	NSError* pipelineError = nil;
	pipelineState = [g->GetDevice() newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&pipelineError];

	return true;
}

PipelineStateMetal::PipelineStateMetal()
{
	shaders.fill(nullptr);
}

PipelineStateMetal::~PipelineStateMetal()
{
	for (auto& shader : shaders)
	{
		SafeRelease(shader);
	}

    if (pipelineStateDescriptor != nullptr)
    {
        [pipelineStateDescriptor release];
        pipelineStateDescriptor = nullptr;
    }

    if (depthStencilState != nullptr)
    {
        [depthStencilState release];
        depthStencilState = nullptr;
    }

    if (pipelineState != nullptr)
    {
        [pipelineState release];
        pipelineState = nullptr;
    }
    
	SafeRelease(graphics_);
}

bool PipelineStateMetal::Initialize(GraphicsMetal* graphics)
{
	SafeAddRef(graphics);
	SafeRelease(graphics_);
	graphics_ = graphics;

	return true;
}

void PipelineStateMetal::SetShader(ShaderStageType stage, Shader* shader)
{
	SafeAddRef(shader);
	SafeRelease(shaders[static_cast<int>(stage)]);
	shaders[static_cast<int>(stage)] = shader;
}

bool PipelineStateMetal::Compile() { return Compile(this, graphics_); }

}
