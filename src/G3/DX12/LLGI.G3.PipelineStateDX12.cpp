
#pragma once

#include "LLGI.G3.PipelineStateDX12.h"
#include "../LLGI.G3.PipelineState.h"
#include "LLGI.G3.ShaderDX12.h"

namespace LLGI
{
namespace G3
{

PipelineStateDX12::PipelineStateDX12(GraphicsDX12* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);
}

PipelineStateDX12::~PipelineStateDX12()
{
	SafeRelease(pixelShader);
	SafeRelease(vertexShader);
	SafeRelease(graphics_);
}

void PipelineStateDX12::SetShader(ShaderStageType stage, Shader* shader)
{
	switch (stage)
	{
	case ShaderStageType::Pixel:
		SafeAddRef(shader);
		pixelShader = CreateSharedPtr(shader);
		break;
	case ShaderStageType::Vertex:
		SafeAddRef(shader);
		vertexShader = CreateSharedPtr(shader);
		break;
	}
}

void PipelineStateDX12::Compile()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};

	auto vs = reinterpret_cast<ShaderDX12*>(vertexShader.get())->GetData();
	pipelineStateDesc.VS.pShaderBytecode = vs->Data;
	pipelineStateDesc.VS.BytecodeLength = vs->Size;

	auto ps = reinterpret_cast<ShaderDX12*>(pixelShader.get())->GetData();
	pipelineStateDesc.PS.pShaderBytecode = vs->Data;
	pipelineStateDesc.PS.BytecodeLength = vs->Size;

	D3D12_INPUT_ELEMENT_DESC elementDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	pipelineStateDesc.InputLayout = {elementDescs, _countof(elementDescs)};
	// pipelineStateDesc.pRootSignature = RootSignature;
	pipelineStateDesc.VS = {vs->Data, (size_t)vs->Size};
	pipelineStateDesc.PS = {ps->Data, (size_t)ps->Size};
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.SampleDesc.Count = 1;

	auto hr = graphics_->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

FAILED_EXIT:
	return;
}

} // namespace G3
} // namespace LLGI
