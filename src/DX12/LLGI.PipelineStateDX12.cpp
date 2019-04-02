
#pragma once

#include "LLGI.PipelineStateDX12.h"
#include "../LLGI.PipelineState.h"
#include "LLGI.ShaderDX12.h"

namespace LLGI
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
	CreateRootSignature();

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
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// レンダーターゲットのブレンド設定.
	D3D12_RENDER_TARGET_BLEND_DESC RTBSDesc = {FALSE,
											   FALSE,
											   D3D12_BLEND_ONE,
											   D3D12_BLEND_ZERO,
											   D3D12_BLEND_OP_ADD,
											   D3D12_BLEND_ONE,
											   D3D12_BLEND_ZERO,
											   D3D12_BLEND_OP_ADD,
											   D3D12_LOGIC_OP_NOOP,
											   D3D12_COLOR_WRITE_ENABLE_ALL};

	// ブレンドステートの設定.
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		blendDesc.RenderTarget[i] = RTBSDesc;
	}

	pipelineStateDesc.InputLayout = {elementDescs, _countof(elementDescs)};
	pipelineStateDesc.pRootSignature = RootSignature_;
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
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;

	auto hr = graphics_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
	SafeAddRef(pipelineState_);

	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

FAILED_EXIT:
	SafeRelease(pipelineState_);
	return;
}

bool PipelineStateDX12::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = 0;
	desc.pParameters = nullptr;
	desc.NumStaticSamplers = 0;
	desc.pStaticSamplers = nullptr;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature_, nullptr);
	SafeAddRef(Signature_);
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = graphics_->GetDevice()->CreateRootSignature(
		0, Signature_->GetBufferPointer(), Signature_->GetBufferSize(), IID_PPV_ARGS(&RootSignature_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
		SafeRelease(Signature_);
	}
	SafeAddRef(RootSignature_);
	return true;

FAILED_EXIT:
	SafeRelease(RootSignature_);
	return false;
}

} // namespace LLGI
