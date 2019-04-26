#pragma once

#include "../LLGI.Graphics.h"
#include "LLGI.BaseDX12.h"

#include <functional>

namespace LLGI
{

class RenderPassDX12 : public RenderPass
{
public:
	Vec2I screenWindowSize;
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV;
	ID3D12Resource* RenderPass;
};

class GraphicsDX12 : public Graphics
{
private:
	ID3D12Device* device_ = nullptr;
	std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc_;
	std::function<void()> waitFunc_;

	const D3D12_COMMAND_LIST_TYPE commandListType_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ID3D12CommandQueue* commandQueue_ = nullptr;
	ID3D12CommandAllocator* commandAllocator_ = nullptr;

	RenderPassDX12 currentScreen;

public:
	GraphicsDX12(ID3D12Device* device,
				 std::function<std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc,
				 std::function<void()> waitFunc,
				 ID3D12CommandQueue* commandQueue);
	virtual ~GraphicsDX12();

	void Execute(CommandList* commandList) override;
	void WaitFinish() override;

	RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;
	VertexBuffer* CreateVertexBuffer(int32_t size) override;
	IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count) override;
	Shader* CreateShader(DataStructure* data, int32_t count) override;
	PipelineState* CreatePiplineState() override;
	CommandList* CreateCommandList() override;

	ID3D12Device* GetDevice();
};

} // namespace LLGI
