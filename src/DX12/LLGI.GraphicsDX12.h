#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.RenderPassDX12.h"
#include "LLGI.RenderPassPipelineStateDX12.h"

#include <functional>
#include <unordered_map>

namespace LLGI
{
class PlatformDX12;
class RenderPassDX12;

class GraphicsDX12 : public Graphics
{
private:
	int32_t swapBufferCount_ = 0;
	int32_t currentSwapBufferIndex = -1;

	ID3D12Device* device_ = nullptr;
	std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, Texture*>()> getScreenFunc_;
	std::function<void()> waitFunc_;

	const D3D12_COMMAND_LIST_TYPE commandListType_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12CommandQueue* commandQueue_ = nullptr;
	ID3D12CommandAllocator* commandAllocator_ = nullptr;
	ReferenceObject* owner_ = nullptr;

	std::unordered_map<RenderPassPipelineStateDX12Key, std::shared_ptr<RenderPassPipelineStateDX12>, RenderPassPipelineStateDX12Key::Hash>
		renderPassPipelineStates;

public:
	GraphicsDX12(ID3D12Device* device,
				 std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, Texture*>()> getScreenFunc,
				 std::function<void()> waitFunc,
				 ID3D12CommandQueue* commandQueue,
				 int32_t swapBufferCount,
				 ReferenceObject* owner = nullptr);
	virtual ~GraphicsDX12();

	void Execute(CommandList* commandList) override;
	void WaitFinish() override;

	VertexBuffer* CreateVertexBuffer(int32_t size) override;
	IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count) override;
	ConstantBuffer* CreateConstantBuffer(int32_t size) override;
	Shader* CreateShader(DataStructure* data, int32_t count) override;
	PipelineState* CreatePiplineState() override;
	SingleFrameMemoryPool* CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount) override;
	CommandList* CreateCommandList(SingleFrameMemoryPool* memoryPool) override;
	RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) override;
	Texture* CreateTexture(uint64_t id) override;
	Texture* CreateTexture(const TextureInitializationParameter& parameter) override;
	Texture* CreateRenderTexture(const RenderTextureInitializationParameter& parameter) override;
	Texture* CreateDepthTexture(const DepthTextureInitializationParameter& parameter) override;

	std::shared_ptr<RenderPassPipelineStateDX12> CreateRenderPassPipelineState(bool isPresentMode,
																			   bool hasDepth,
																			   int32_t renderTargetCount,
																			   std::array<DXGI_FORMAT, 8> renderTargetFormats);

	RenderPassPipelineState* CreateRenderPassPipelineState(RenderPass* renderpass) override;

	RenderPassPipelineState* CreateRenderPassPipelineState(const RenderPassPipelineStateKey& key) override;

	ID3D12Device* GetDevice();

	// int32_t GetCurrentSwapBufferIndex() const;
	int32_t GetSwapBufferCount() const;
	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_; }

	ID3D12Resource* CreateResource(D3D12_HEAP_TYPE heapType,
								   DXGI_FORMAT format,
								   D3D12_RESOURCE_DIMENSION resourceDimention,
								   D3D12_RESOURCE_STATES resourceState,
								   Vec2I size)
	{
		return CreateResource(heapType, format, resourceDimention, resourceState, D3D12_RESOURCE_FLAG_NONE, size);
	}

	ID3D12Resource* CreateResource(D3D12_HEAP_TYPE heapType,
								   DXGI_FORMAT format,
								   D3D12_RESOURCE_DIMENSION resourceDimention,
								   D3D12_RESOURCE_STATES resourceState,
								   D3D12_RESOURCE_FLAGS flags,
								   Vec2I size);

	std::vector<uint8_t> CaptureRenderTarget(Texture* renderTarget);
};

} // namespace LLGI
