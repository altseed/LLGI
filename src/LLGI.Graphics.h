
#pragma once

#include "LLGI.Base.h"
#include "Utils/LLGI.FixedSizeVector.h"
#include <functional>
#include <unordered_map>

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
	bool IsMultiSampling = false;
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
protected:
	int32_t currentSwapBuffer_ = -1;
	int32_t swapBufferCount_ = 0;
	std::vector<int32_t> offsets_;
	std::vector<std::vector<ConstantBuffer*>> constantBuffers_;

	/**
		@brief	create constant buffer
	*/
	virtual ConstantBuffer* CreateConstantBufferInternal(int32_t size) { return nullptr; }

	/**
		@brief	reinitialize buffer with a size
	*/
	virtual ConstantBuffer* ReinitializeConstantBuffer(ConstantBuffer* cb, int32_t size) { return nullptr; }

public:
	SingleFrameMemoryPool(int32_t swapBufferCount = 3);
	virtual ~SingleFrameMemoryPool();

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

	FixedSizeVector<Texture*, RenderTargetMax> renderTextures_;
	Texture* depthTexture_ = nullptr;

protected:
	Vec2I screenSize_;

	bool assignRenderTextures(Texture** textures, int32_t count);
	bool assignDepthTexture(Texture* depthTexture);

	bool getSize(Vec2I& size, const Texture** textures, int32_t textureCount, Texture* depthTexture) const;

public:
	RenderPass() = default;
	virtual ~RenderPass();

	virtual bool GetIsColorCleared() const { return isColorCleared_; }

	virtual bool GetIsDepthCleared() const { return isDepthCleared_; }

	virtual Color8 GetClearColor() const { return color_; }

	virtual void SetIsColorCleared(bool isColorCleared);

	virtual void SetIsDepthCleared(bool isDepthCleared);

	virtual void SetClearColor(const Color8& color);

	virtual Texture* GetRenderTexture(int index) const { return renderTextures_.at(index); }

	virtual int GetRenderTextureCount() const { return static_cast<int32_t>(renderTextures_.size()); }

	Texture* GetDepthTexture() const { return depthTexture_; }

	bool GetHasDepthTexture() const { return GetDepthTexture() != nullptr; }

	virtual bool GetIsSwapchainScreen() const;

	virtual Vec2I GetScreenSize() const { return screenSize_; }
};

struct RenderPassPipelineStateKey
{
	bool IsPresent = false;
	bool HasDepth = false;
	FixedSizeVector<TextureFormatType, RenderTargetMax> RenderTargetFormats;
	bool IsColorCleared = true;
	bool IsDepthCleared = true;

	bool operator==(const RenderPassPipelineStateKey& value) const
	{
		if (RenderTargetFormats.size() != value.RenderTargetFormats.size())
			return false;

		for (size_t i = 0; i < RenderTargetFormats.size(); i++)
		{
			if (RenderTargetFormats.at(i) != value.RenderTargetFormats.at(i))
				return false;
		}

		return (IsPresent == value.IsPresent && HasDepth == value.HasDepth && IsColorCleared == value.IsColorCleared && IsDepthCleared == value.IsDepthCleared);
	}

	struct Hash
	{
		typedef std::size_t result_type;

		std::size_t operator()(const RenderPassPipelineStateKey& key) const
		{
			auto ret = std::hash<bool>()(key.IsPresent);
			ret += std::hash<bool>()(key.HasDepth);
			ret += std::hash<bool>()(key.IsColorCleared);
			ret += std::hash<bool>()(key.IsDepthCleared);

			for (int32_t i = 0; i < key.RenderTargetFormats.size(); i++)
			{
				ret += std::hash<uint64_t>()((uint64_t)key.RenderTargetFormats.at(i));
			}

			return ret;
		}
	};
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

	/*[[deprecated("use Platform::SetWindowSize.")]]*/ virtual void SetWindowSize(const Vec2I& windowSize);

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
        @param  drawingCount(drawingCount is ignored in DirectX12)
	*/
	virtual SingleFrameMemoryPool* CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount);

	/**
		@brief
		@param memoryPool if memory pool is null, allocate memory from graphics
	*/
	virtual CommandList* CreateCommandList(SingleFrameMemoryPool* memoryPool);

	/**
		@brief	create a constant buffer
		@param	size buffer size
	*/
	virtual ConstantBuffer* CreateConstantBuffer(int32_t size);

	virtual RenderPass* CreateRenderPass(const Texture** textures, int32_t textureCount, Texture* depthTexture) { return nullptr; }

	virtual Texture* CreateTexture(const TextureInitializationParameter& parameter) { return nullptr; }

	virtual Texture* CreateRenderTexture(const RenderTextureInitializationParameter& parameter) { return nullptr; }

	virtual Texture* CreateDepthTexture(const DepthTextureInitializationParameter& parameter) { return nullptr; }

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

	/**
		@brief	create a RenderPassPipelineState
		@note
		This is a function to create an object.
		But it is very fast. So it can call it in everyframe.
	*/
	virtual RenderPassPipelineState* CreateRenderPassPipelineState(const RenderPassPipelineStateKey& key) { return nullptr; }

	/** For testing. Wait for all commands in queue to complete. Then read data from specified render target. */
	virtual std::vector<uint8_t> CaptureRenderTarget(Texture* renderTarget);

	/**
		@brief	specify a function which is called when this instance is disposed.
		@param	disposed	called function
	*/
	void SetDisposed(const std::function<void()>& disposed);
};

} // namespace LLGI
