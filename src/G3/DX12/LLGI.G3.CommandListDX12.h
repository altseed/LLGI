
#pragma once

#include "../LLGI.G3.CommandList.h"
#include "LLGI.G3.BaseDX12.h"
#include "LLGI.G3.GraphicsDX12.h"

namespace LLGI
{
namespace G3
{

class CommandListDX12
	: public CommandList
{
private:
	std::shared_ptr<GraphicsDX12> graphics_;
	std::shared_ptr<RenderPassDX12> renderPass_;

	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	
public:
	CommandListDX12();
	virtual ~CommandListDX12();
	bool Initialize(GraphicsDX12* graphics);

	void Begin() override;
	void End() override;
	void Clear(const Color8& color);
	void BeginRenderPass(RenderPass* renderPass) override;
	void EndRenderPass() override;

	ID3D12GraphicsCommandList* GetCommandList() { return commandList; }
};

}
}

