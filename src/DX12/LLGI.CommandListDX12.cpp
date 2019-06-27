
#include "LLGI.CommandListDX12.h"
#include "LLGI.ConstantBufferDX12.h"
#include "LLGI.GraphicsDX12.h"
#include "LLGI.IndexBufferDX12.h"
#include "LLGI.PipelineStateDX12.h"
#include "LLGI.TextureDX12.h"
#include "LLGI.VertexBufferDX12.h"

namespace LLGI
{

CommandListDX12::CommandListDX12() {}

CommandListDX12::~CommandListDX12() { descriptorHeaps_.clear(); }

bool CommandListDX12::Initialize(GraphicsDX12* graphics)
{
	HRESULT hr;

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	for (int32_t i = 0; i < graphics_->GetSwapBufferCount(); i++)
	{
		ID3D12CommandAllocator* commandAllocator = nullptr;
		ID3D12GraphicsCommandList* commandList = nullptr;

		hr = graphics_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
		if (FAILED(hr))
		{
			goto FAILED_EXIT;
		}
		commandAllocators.push_back(CreateSharedPtr(commandAllocator));

		hr = graphics_->GetDevice()->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
		if (FAILED(hr))
		{
			goto FAILED_EXIT;
		}
		commandList->Close();
		commandLists.push_back(CreateSharedPtr(commandList));
	}

	for (size_t i = 0; i < static_cast<size_t>(graphics_->GetSwapBufferCount()); i++)
	{
		auto dp = std::make_shared<DescriptorHeapDX12>(graphics_, 100, 2);
		descriptorHeaps_.push_back(dp);
	}

	return true;

FAILED_EXIT:;
	graphics_.reset();
	commandAllocators.clear();
	commandLists.clear();
	descriptorHeaps_.clear();

	return false;
}

void CommandListDX12::Begin()
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];
	commandList->Reset(commandAllocators[graphics_->GetCurrentSwapBufferIndex()].get(), nullptr);

	auto& dp = descriptorHeaps_[graphics_->GetCurrentSwapBufferIndex()];
	dp->Reset();

	CommandList::Begin();
}

void CommandListDX12::End()
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];

	commandList->Close();
}

void CommandListDX12::BeginRenderPass(RenderPass* renderPass)
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];

	SafeAddRef(renderPass);
	renderPass_ = CreateSharedPtr((RenderPassDX12*)renderPass);

	if (renderPass != nullptr)
	{
		if (!renderPass_->GetIsScreen())
		{
			SetResourceBarrier(renderPass_->renderPass_, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			auto& descriptorHeaps = descriptorHeaps_[graphics_->GetCurrentSwapBufferIndex()];
			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			auto cpuHandle = descriptorHeaps->GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			graphics_->GetDevice()->CreateRenderTargetView(renderPass_->renderPass_, &desc, cpuHandle);
			renderPass_->handleRtv_ = cpuHandle;
			descriptorHeaps->IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
			descriptorHeaps->IncrementGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);

		}

		// Set render target
		commandList->OMSetRenderTargets(1, &(renderPass_->handleRtv_), FALSE, nullptr);

		// TODO depth...

		// Reset scissor
		D3D12_RECT rect;
		rect.top = 0;
		rect.left = 0;
		rect.right = renderPass_->screenWindowSize.X;
		rect.bottom = renderPass_->screenWindowSize.Y;
		commandList->RSSetScissorRects(1, &rect);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = renderPass_->screenWindowSize.X;
		viewport.Height = renderPass_->screenWindowSize.Y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		commandList->RSSetViewports(1, &viewport);

		// Clear color
		if (renderPass_->GetIsColorCleared())
		{
			Clear(renderPass_->GetClearColor());
		}
	}
}

void CommandListDX12::EndRenderPass()
{
	if (!renderPass_->GetIsScreen())
	{
		SetResourceBarrier(renderPass_->renderPass_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	renderPass_.reset();
}

void CommandListDX12::Draw(int32_t pritimiveCount)
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];

	BindingVertexBuffer vb_;
	IndexBuffer* ib_ = nullptr;
	ConstantBuffer* cb = nullptr;
	PipelineState* pip_ = nullptr;

	bool isVBDirtied = false;
	bool isIBDirtied = false;
	bool isPipDirtied = false;

	GetCurrentVertexBuffer(vb_, isVBDirtied);
	GetCurrentIndexBuffer(ib_, isIBDirtied);
	GetCurrentPipelineState(pip_, isPipDirtied);

	assert(vb_.vertexBuffer != nullptr);
	assert(ib_ != nullptr);
	assert(pip_ != nullptr);

	auto vb = static_cast<VertexBufferDX12*>(vb_.vertexBuffer);
	auto ib = static_cast<IndexBufferDX12*>(ib_);
	auto pip = static_cast<PipelineStateDX12*>(pip_);

	{
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		vertexView.BufferLocation = vb->Get()->GetGPUVirtualAddress() + vb_.offset;
		vertexView.StrideInBytes = vb_.stride;
		vertexView.SizeInBytes = vb_.vertexBuffer->GetSize() - vb_.offset;
		if (vb_.vertexBuffer != nullptr)
		{
			commandList->IASetVertexBuffers(0, 1, &vertexView);
		}
	}

	if (ib != nullptr)
	{
		D3D12_INDEX_BUFFER_VIEW indexView;
		indexView.BufferLocation = ib->Get()->GetGPUVirtualAddress();
		indexView.SizeInBytes = ib->GetStride() * ib->GetCount();
		indexView.Format = ib->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		commandList->IASetIndexBuffer(&indexView);
	}

	if (pip != nullptr)
	{
		commandList->SetGraphicsRootSignature(pip->GetRootSignature());
		auto p = pip->GetPipelineState();
		commandList->SetPipelineState(p);
	}

	auto& descriptorHeaps = descriptorHeaps_[graphics_->GetCurrentSwapBufferIndex()];
	{
		// set using descriptor heaps
		ID3D12DescriptorHeap* heaps[] = {
			descriptorHeaps->GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), // descriptor heap for CBV, SRV and UAV
			descriptorHeaps->GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),	 // descriptor heap for Sampler
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// set descriptor tables
		commandList->SetGraphicsRootDescriptorTable(0, descriptorHeaps->GetGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		commandList->SetGraphicsRootDescriptorTable(1, descriptorHeaps->GetGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));
	}

	int increment = NumTexture * static_cast<int>(ShaderStageType::Max);

	// constant buffer
	{
		for (int stage_ind = 0; stage_ind < static_cast<int>(ShaderStageType::Max); stage_ind++)
		{
			GetCurrentConstantBuffer(static_cast<ShaderStageType>(stage_ind), cb);
			if (cb != nullptr)
			{
				auto _cb = static_cast<ConstantBufferDX12*>(cb);
				D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
				desc.BufferLocation = _cb->Get()->GetGPUVirtualAddress();
				desc.SizeInBytes = _cb->GetSize();
				auto cpuHandle = descriptorHeaps->GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				graphics_->GetDevice()->CreateConstantBufferView(&desc, cpuHandle);
			}
			descriptorHeaps->IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		}
		descriptorHeaps->IncrementGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, static_cast<int>(ShaderStageType::Max));
	}

	{
		for (int stage_ind = 0; stage_ind < static_cast<int>(ShaderStageType::Max); stage_ind++)
		{
			for (int unit_ind = 0; unit_ind < currentTextures[stage_ind].size(); unit_ind++)
			{
				if (currentTextures[stage_ind][unit_ind].texture != nullptr)
				{
					auto texture = static_cast<TextureDX12*>(currentTextures[stage_ind][unit_ind].texture);
					auto wrapMode = currentTextures[stage_ind][unit_ind].wrapMode;
					auto minMagFilter = currentTextures[stage_ind][unit_ind].minMagFilter;

					// SRV
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						srvDesc.Texture2D.MipLevels = 1;
						srvDesc.Texture2D.MostDetailedMip = 0;

						auto cpuHandle = descriptorHeaps->GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						graphics_->GetDevice()->CreateShaderResourceView(texture->Get(), &srvDesc, cpuHandle);
					}

					// Sampler
					{
						D3D12_SAMPLER_DESC samplerDesc = {};

						// TODO
						samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

						if (wrapMode == TextureWrapMode::Repeat)
						{
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						}
						else
						{
							samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
							samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
						}
						samplerDesc.MipLODBias = 0;
						samplerDesc.MaxAnisotropy = 0;
						samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
						samplerDesc.MinLOD = 0.0f;
						samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

						auto cpuHandle = descriptorHeaps->GetCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
						graphics_->GetDevice()->CreateSampler(&samplerDesc, cpuHandle);
					}
				}
				descriptorHeaps->IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
				descriptorHeaps->IncrementCpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1);
			}
		}
		descriptorHeaps->IncrementGpuHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, increment);
	}

	// setup a topology (triangle)
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw polygon
	commandList->DrawIndexedInstanced(pritimiveCount * 3 /*triangle*/, 1, 0, 0, 0);

	CommandList::Draw(pritimiveCount);
}

void CommandListDX12::Clear(const Color8& color)
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];

	auto rt = renderPass_;
	if (rt == nullptr)
		return;

	float color_[] = {color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f};

	commandList->ClearRenderTargetView(rt->handleRtv_, color_, 0, nullptr);
}

ID3D12GraphicsCommandList* CommandListDX12::GetCommandList() const
{
	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];
	return commandList.get();
}

void CommandListDX12::SetResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	auto commandList = commandLists[graphics_->GetCurrentSwapBufferIndex()];
	commandList->ResourceBarrier(1, &barrier);
}

} // namespace LLGI
