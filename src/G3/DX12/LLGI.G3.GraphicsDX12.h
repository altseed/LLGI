#pragma once

#include "../LLGI.G3.Graphics.h"
#include "LLGI.G3.BaseDX12.h"

#include <functional>

namespace LLGI
{
namespace G3
{

class RenderPassDX12
	: public RenderPass
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV;
	ID3D12Resource* RenderPass;
};

class GraphicsDX12
	: public Graphics
{
private:
	ID3D12Device* device_ = nullptr;
	std::function<std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc_;

		;
	ID3D12CommandQueue* commandQueue_ = nullptr;

	RenderPassDX12 currentScreen;

public:
	GraphicsDX12(ID3D12Device* device, std::function<std::tuple< D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*>()> getScreenFunc, ID3D12CommandQueue* commandQueue);
	virtual ~GraphicsDX12();

	void Execute(CommandList* commandList) override;

	RenderPass* GetCurrentScreen() override;
	VertexBuffer* CreateVertexBuffer(int32_t size) override;
	CommandList* CreateCommandList() override;

	ID3D12Device* GetDevice();
};
	

}
}