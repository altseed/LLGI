
#include "LLGI.CommandListDX12.h"
#include "LLGI.ConstantBufferDX12.h"
#include "LLGI.GraphicsDX12.h"
#include "LLGI.IndexBufferDX12.h"
#include "LLGI.PipelineStateDX12.h"
#include "LLGI.RenderPassDX12.h"
#include "LLGI.TextureDX12.h"
#include "LLGI.VertexBufferDX12.h"

namespace LLGI
{
CommandListDX12::CommandListDX12() {}

CommandListDX12::~CommandListDX12() { swapBuffers_.clear(); }

bool CommandListDX12::Initialize(GraphicsDX12* graphics, int32_t drawingCount)
{
	HRESULT hr;

	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);

	swapBuffers_.resize(graphics_->GetSwapBufferCount());

	for (int32_t i = 0; i < graphics_->GetSwapBufferCount(); i++)
	{
		ID3D12CommandAllocator* commandAllocator = nullptr;
		ID3D12GraphicsCommandList* commandList = nullptr;

		hr = graphics_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
		if (FAILED(hr))
		{
			goto FAILED_EXIT;
		}
		swapBuffers_[i].commandAllocator = CreateSharedPtr(commandAllocator);

		hr = graphics_->GetDevice()->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
		if (FAILED(hr))
		{
			goto FAILED_EXIT;
		}
		commandList->Close();
		swapBuffers_[i].commandList = CreateSharedPtr(commandList);

		swapBuffers_[i].cbreDescriptorHeap = std::make_shared<DescriptorHeapDX12>(
			graphics_, D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, drawingCount * NumTexture + 2, 2);

		// the maximum render target is defined temporary
		swapBuffers_[i].rtDescriptorHeap = std::make_shared<DescriptorHeapDX12>(
			graphics_, D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV, drawingCount / 2, 2);
		swapBuffers_[i].smpDescriptorHeap = std::make_shared<DescriptorHeapDX12>(
			graphics_, D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, drawingCount * NumTexture, 2);
	}

	return true;

FAILED_EXIT:;
	graphics_.reset();
	swapBuffers_.clear();
	return false;
}

void CommandListDX12::Begin()
{
	currentSwap_++;
	currentSwap_ %= swapBuffers_.size();

	auto& swapBuffer = swapBuffers_[currentSwap_];
	auto commandList = swapBuffer.commandList;
	commandList->Reset(swapBuffer.commandAllocator.get(), nullptr);

	swapBuffer.cbreDescriptorHeap->Reset();
	swapBuffer.rtDescriptorHeap->Reset();
	swapBuffer.smpDescriptorHeap->Reset();

	CommandList::Begin();
}

void CommandListDX12::End()
{
	auto& swapBuffer = swapBuffers_[currentSwap_];
	swapBuffer.commandList->Close();
}

void CommandListDX12::BeginRenderPass(RenderPass* renderPass)
{
	assert(currentSwap_ >= 0);

	auto& swapBuffer = swapBuffers_[currentSwap_];
	auto commandList = swapBuffer.commandList;

	SafeAddRef(renderPass);
	renderPass_ = CreateSharedPtr((RenderPassDX12*)renderPass);

	if (renderPass != nullptr)
	{
		if (!renderPass_->GetIsScreen())
		{
			auto r = renderPass_->CreateRenderTargetViews(this, swapBuffer.rtDescriptorHeap.get());
			assert(r);
		}

		// Set render target
		commandList->OMSetRenderTargets(renderPass_->GetCount(), renderPass_->GetHandleRTV(), FALSE, nullptr);

		// TODO depth...

		// Reset scissor
		D3D12_RECT rect;
		rect.top = 0;
		rect.left = 0;
		rect.right = renderPass_->GetScreenWindowSize().X;
		rect.bottom = renderPass_->GetScreenWindowSize().Y;
		commandList->RSSetScissorRects(1, &rect);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(renderPass_->GetScreenWindowSize().X);
		viewport.Height = static_cast<float>(renderPass_->GetScreenWindowSize().Y);
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

void CommandListDX12::EndRenderPass() { renderPass_.reset(); }

void CommandListDX12::Draw(int32_t pritimiveCount)
{
	auto& swapBuffer = swapBuffers_[currentSwap_];
	auto commandList = swapBuffer.commandList;

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

	{
		// set using descriptor heaps
		ID3D12DescriptorHeap* heaps[] = {
			swapBuffer.cbreDescriptorHeap->GetHeap(),
			swapBuffer.smpDescriptorHeap->GetHeap(),
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// set descriptor tables
		commandList->SetGraphicsRootDescriptorTable(0, swapBuffer.cbreDescriptorHeap->GetGpuHandle());
		commandList->SetGraphicsRootDescriptorTable(1, swapBuffer.smpDescriptorHeap->GetGpuHandle());
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
				desc.BufferLocation = _cb->Get()->GetGPUVirtualAddress() + _cb->GetOffset();
				desc.SizeInBytes = _cb->GetActualSize();
				auto cpuHandle = swapBuffer.cbreDescriptorHeap->GetCpuHandle();
				graphics_->GetDevice()->CreateConstantBufferView(&desc, cpuHandle);
			}
			else
			{
				// set dummy values
				D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
				desc.BufferLocation = D3D12_GPU_VIRTUAL_ADDRESS();
				desc.SizeInBytes = 0;
				auto cpuHandle = swapBuffer.cbreDescriptorHeap->GetCpuHandle();
				graphics_->GetDevice()->CreateConstantBufferView(&desc, cpuHandle);
			}
			swapBuffer.cbreDescriptorHeap->IncrementCpuHandle(1);
		}
		swapBuffer.cbreDescriptorHeap->IncrementGpuHandle(static_cast<int>(ShaderStageType::Max));
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

					// Make barrior to use a render target
					if (texture->IsRenderTexture())
					{
						if (stage_ind == static_cast<int>(ShaderStageType::Pixel))
						{
							texture->ResourceBarrior(commandList.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						}
						else
						{
							texture->ResourceBarrior(commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
						}
					}

					// SRV
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						srvDesc.Texture2D.MipLevels = 1;
						srvDesc.Texture2D.MostDetailedMip = 0;

						auto cpuHandle = swapBuffer.cbreDescriptorHeap->GetCpuHandle();
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

						auto cpuHandle = swapBuffer.smpDescriptorHeap->GetCpuHandle();
						graphics_->GetDevice()->CreateSampler(&samplerDesc, cpuHandle);
					}
				}
				swapBuffer.cbreDescriptorHeap->IncrementCpuHandle(1);
				swapBuffer.smpDescriptorHeap->IncrementCpuHandle(1);
			}
		}
		swapBuffer.cbreDescriptorHeap->IncrementGpuHandle(increment);
		swapBuffer.smpDescriptorHeap->IncrementGpuHandle(increment);
	}

	// setup a topology (triangle)
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw polygon
	commandList->DrawIndexedInstanced(pritimiveCount * 3 /*triangle*/, 1, 0, 0, 0);

	CommandList::Draw(pritimiveCount);
}

void CommandListDX12::Clear(const Color8& color)
{
	auto& swapBuffer = swapBuffers_[currentSwap_];
	auto commandList = swapBuffer.commandList;

	auto rt = renderPass_;
	if (rt == nullptr)
		return;

	float color_[] = {color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f};

	auto handle = rt->GetHandleRTV();
	for (int i = 0; i < rt->GetCount(); i++)
	{
		commandList->ClearRenderTargetView(handle[i], color_, 0, nullptr);
	}
}

ID3D12GraphicsCommandList* CommandListDX12::GetCommandList() const
{
	auto& swapBuffer = swapBuffers_[currentSwap_];
	auto commandList = swapBuffer.commandList;

	return commandList.get();
}

} // namespace LLGI
