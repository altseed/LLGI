#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"
#include "LLGI.RenderPassDX12.h"
#include "LLGI.RenderPassPipelineStateDX12.h"

#include <functional>
#include <unordered_map>

namespace LLGI
{
class RenderPassDX12;
class RenderPassPipelineStateDX12Key;
class RenderPassPipelineStateDX12Key::Hash;

class GraphicsDX12 : public Graphics
{
private:
	int32_t swapBufferCount_ = 0;
	int32_t currentSwapBufferIndex = -1;

	ID3D12Device* device_ = nullptr;
	std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc_;
	std::function<void()> waitFunc_;

	const D3D12_COMMAND_LIST_TYPE commandListType_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12CommandQueue* commandQueue_ = nullptr;
	ID3D12CommandAllocator* commandAllocator_ = nullptr;

	RenderPassDX12 currentScreen;

	std::unordered_map<RenderPassPipelineStateDX12Key, std::weak_ptr<RenderPassPipelineStateDX12>, RenderPassPipelineStateDX12Key::Hash>
		renderPassPipelineStates;

public:
	GraphicsDX12(ID3D12Device* device,
				 std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc,
				 std::function<void()> waitFunc,
				 ID3D12CommandQueue* commandQueue,
				 int32_t swapBufferCount);
	virtual ~GraphicsDX12();

	void NewFrame() override;

	void Execute(CommandList* commandList) override;
	void WaitFinish() override;

	RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;
	VertexBuffer* CreateVertexBuffer(int32_t size) override;
	IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count) override;
	ConstantBuffer* CreateConstantBuffer(int32_t size, ConstantBufferType type = ConstantBufferType::LongTime) override;
	Shader* CreateShader(DataStructure* data, int32_t count) override;
	PipelineState* CreatePiplineState() override;
	CommandList* CreateCommandList() override;
	RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) override;
	Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) override;
	Texture* CreateTexture(uint64_t id) override;

	std::shared_ptr<RenderPassPipelineStateDX12> CreateRenderPassPipelineState(bool isPresentMode, bool hasDepth);

	ID3D12Device* GetDevice();

	int32_t GetCurrentSwapBufferIndex() const;
	int32_t GetSwapBufferCount() const;
	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_; }

	ID3D12Resource* CreateResource(D3D12_HEAP_TYPE heapType,
								   DXGI_FORMAT format,
								   D3D12_RESOURCE_DIMENSION resourceDimention,
								   D3D12_RESOURCE_STATES resourceState,
								   Vec2I size);
	ID3D12DescriptorHeap* CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
};

} // namespace LLGI
