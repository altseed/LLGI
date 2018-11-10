
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class RenderPass
	: public ReferenceObject
{
private:
	bool isColorCleared_ = false;

	bool isDepthCleared_ = false;

	Color8 color_;

public:
	RenderPass() = default;
	virtual ~RenderPass() = default;

	virtual bool GetIsColorCleared() const { return isColorCleared_; }
				 
	virtual bool GetIsDepthCleared() const { return isDepthCleared_; }
				 
	virtual Color8 GetClearColor() const { return color_; }

	virtual void SetIsColorCleared(bool isColorCleared);

	virtual void SetIsDepthCleared(bool isDepthCleared);

	virtual void SetClearColor(const Color8& color);
};

class Graphics
	: public ReferenceObject
{
protected:
	Vec2I windowSize_;

public:
	Graphics() = default;
	virtual ~Graphics() = default;

	/**
		@brief	Start new frame
	*/
	virtual void NewFrame();
	virtual void SetWindowSize(const Vec2I& windowSize);

	virtual void Execute(CommandList* commandList);

	/**
	@brief	to prevent instances to be disposed before finish rendering, finish all renderings.
	*/
	virtual void WaitFinish() {}

	/**
		@brief get render pass of screen to show on a display.
		@note
		Don't release and addref it.
		Don't use it for the many purposes, please input Clear or SetRenderPass immediately.
	*/
	virtual RenderPass* GetCurrentScreen(const Color8& clearColor = Color8(), bool isColorCleared = false, bool isDepthCleared = false);
	virtual VertexBuffer* CreateVertexBuffer(int32_t size);
	virtual IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count);
	virtual Shader* CreateShader(DataStructure* data, int32_t count);
	virtual PipelineState* CreatePiplineState();
	virtual CommandList* CreateCommandList();
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size, ConstantBufferType type = ConstantBufferType::LongTime);
	virtual RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) { return nullptr; }
	virtual Texture* CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer);
	virtual Texture* CreateTexture(uint64_t id);
};

}
}