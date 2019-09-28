
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{
class CommandListPool;

struct TextureInitializationParameter
{
	Vec2I Size;
	TextureFormatType Format = TextureFormatType::R8G8B8A8_UNORM;
};

struct RenderTextureInitializationParameter
{
	Vec2I Size;
	TextureFormatType Format = TextureFormatType::R8G8B8A8_UNORM;
};

struct DepthTextureInitializationParameter
{
	Vec2I Size;
};

/**
	@brief	provide a memory which is available in one frame
*/
class SingleFrameMemoryPool : public ReferenceObject
{
public:
	SingleFrameMemoryPool() = default;
	virtual ~SingleFrameMemoryPool() = default;

	/**
	@brief	Start new frame
	*/
	virtual void NewFrame();

	virtual ConstantBuffer* CreateConstantBuffer(int32_t size);
};

class RenderPass : public ReferenceObject
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

	virtual Texture* GetColorBuffer(int index);
};

/**
	@brief	A class to send a data for RenderPass
*/
class RenderPassPipelineState : public ReferenceObject
{
private:
public:
	RenderPassPipelineState() = default;
	virtual ~RenderPassPipelineState() = default;
};

/**
	@note
	please call WaitFinish before releasing
*/
class Graphics : public ReferenceObject
{
protected:
	Vec2I windowSize_;
	std::function<void()> disposed_;

public:
	Graphics() = default;
	virtual ~Graphics();

	[[deprecated("use Platform::SetWindowSize.")]] virtual void SetWindowSize(const Vec2I& windowSize);

	/**
		@brief	Execute commands
		@note
		Don't release before finish executing commands.
	*/
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
	//[[deprecated("use Platform::GetCurrentScreen.")]] virtual RenderPass*
	//GetCurrentScreen(const Color8& clearColor = Color8(), bool isColorCleared = false, bool isDepthCleared = false);

	/**
		@brief	create a vertex buffer
		@param	size	the size of vertex buffer
	*/
	virtual VertexBuffer* CreateVertexBuffer(int32_t size);

	/**
		@brief	create an index buffer
		@param	stride	the stride of index(2 or 4)
		@param	count	the number of index
	*/
	virtual IndexBuffer* CreateIndexBuffer(int32_t stride, int32_t count);
	virtual Shader* CreateShader(DataStructure* data, int32_t count);
	virtual PipelineState* CreatePiplineState();

	/**
		@brief create a memory pool
	*/
	[[deprecated("use Graphics::CreateCommandListPool.")]] virtual SingleFrameMemoryPool*
	CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount);

	/**
		@brief
		@param memoryPool if memory pool is null, allocate memory from graphics
	*/
	[[deprecated("use Graphics::CreateCommandListPool.")]] virtual CommandList* CreateCommandList(SingleFrameMemoryPool* memoryPool);

	// TODO: see https://github.com/altseed/LLGI/issues/26
	virtual CommandListPool* CreateCommandListPool(int32_t constantBufferPoolSize, int32_t drawingCount, int32_t swapbufferCount);

	/**
		@brief	create a constant buffer
		@param	size buffer size
	*/
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size);

	virtual RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) { return nullptr; }

	virtual Texture* CreateTexture(const TextureInitializationParameter& parameter) { return nullptr; }

	virtual Texture* CreateRenderTexture(const RenderTextureInitializationParameter& parameter) { return nullptr; }

	virtual Texture* CreateDepthTexture(const DepthTextureInitializationParameter& parameter) { return nullptr; }

	[[deprecated("please use CreateTexture, CreateRenderTexture and CreateDepthTexture")]] virtual Texture*
	CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer);

	/**
		@brief	create texture from pointer or id in current platform
	*/
	virtual Texture* CreateTexture(uint64_t id);

	/**
		@brief	create a RenderPassPipelineState
		@note
		This is a function to create an object.
		But it is very fast. So it can call it in everyframe.
	*/
	virtual RenderPassPipelineState* CreateRenderPassPipelineState(RenderPass* renderPass);

	/** For testing. Wait for all commands in queue to complete. Then read data from specified render target. */
	virtual std::vector<uint8_t> CaptureRenderTarget(Texture* renderTarget);

	/**
		@brief	specify a function which is called when this instance is disposed.
		@param	disposed	called function
	*/
	void SetDisposed(const std::function<void()>& disposed);
};

} // namespace LLGI
