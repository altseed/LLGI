
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

PipelineStateDX12::~PipelineStateDX12() {}

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

	auto& vs = reinterpret_cast<ShaderDX12*>(vertexShader.get())->GetData();
	pipelineStateDesc.VS.pShaderBytecode = vs.data();
	pipelineStateDesc.VS.BytecodeLength = vs.size();

	auto& ps = reinterpret_cast<ShaderDX12*>(pixelShader.get())->GetData();
	pipelineStateDesc.PS.pShaderBytecode = ps.data();
	pipelineStateDesc.PS.BytecodeLength = ps.size();

	// setup a vertex layout
	std::array<D3D12_INPUT_ELEMENT_DESC, 16> elementDescs;
	elementDescs.fill(D3D12_INPUT_ELEMENT_DESC{});
	int32_t elementOffset = 0;

	for (int i = 0; i < VertexLayoutCount; i++)
	{
		elementDescs[i].SemanticName = this->VertexLayoutNames[i].c_str();
		elementDescs[i].SemanticIndex = 0;
		elementDescs[i].AlignedByteOffset = elementOffset;
		elementDescs[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			elementOffset += sizeof(float) * 2;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32B32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			elementOffset += sizeof(float) * 3;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32B32A32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			elementOffset += sizeof(float) * 4;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UNORM)
		{
			elementDescs[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			elementOffset += sizeof(float) * 1;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UINT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
			elementOffset += sizeof(float) * 1;
		}
	}

	// TODO...(generate from parameters)
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

	pipelineStateDesc.InputLayout.pInputElementDescs = elementDescs.data();
	pipelineStateDesc.InputLayout.NumElements = VertexLayoutCount;
	pipelineStateDesc.pRootSignature = RootSignature_;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;

	// setup a depth
	pipelineStateDesc.DepthStencilState.DepthEnable = this->IsDepthTestEnabled;
	pipelineStateDesc.DepthStencilState.StencilEnable = this->IsDepthTestEnabled;
	pipelineStateDesc.SampleMask = UINT_MAX;

	// TODO
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// TODO (from renderpass)
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

	return;

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
