#include "LLGI.GraphicsWebGPU.h"

#include "LLGI.BufferWebGPU.h"
#include "LLGI.CommandListWebGPU.h"
#include "LLGI.PipelineStateWebGPU.h"
#include "LLGI.RenderPassPipelineStateWebGPU.h"
#include "LLGI.RenderPassWebGPU.h"
#include "LLGI.ShaderWebGPU.h"
#include "LLGI.TextureWebGPU.h"

namespace LLGI
{

GraphicsWebGPU::GraphicsWebGPU(wgpu::Device device) : device_(device) { queue_ = device.GetQueue(); }

void GraphicsWebGPU::SetWindowSize(const Vec2I& windowSize) { throw "Not implemented"; }

void GraphicsWebGPU::Execute(CommandList* commandList)
{
	auto commandListWgpu = static_cast<CommandListWebGPU*>(commandList);
	auto cb = commandListWgpu->GetCommandBuffer();
	queue_.Submit(1, &cb);
}

void GraphicsWebGPU::WaitFinish() { throw "Not implemented"; }

Buffer* GraphicsWebGPU::CreateBuffer(BufferUsageType usage, int32_t size)
{
	auto obj = new BufferWebGPU();
	if (!obj->Initialize(GetDevice(), usage, size))
	{
		SafeRelease(obj);
		return nullptr;
	}

	return obj;
}

Shader* GraphicsWebGPU::CreateShader(DataStructure* data, int32_t count)
{
	auto obj = new ShaderWebGPU();
	if (!obj->Initialize(GetDevice(), data, count))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

PipelineState* GraphicsWebGPU::CreatePiplineState()
{
	auto pipelineState = new PipelineStateWebGPU(GetDevice());

	// TODO : error check
	return pipelineState;
}

SingleFrameMemoryPool* GraphicsWebGPU::CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount)
{
	throw "Not implemented";
}

CommandList* GraphicsWebGPU::CreateCommandList(SingleFrameMemoryPool* memoryPool)
{
	auto commandList = new CommandListWebGPU(GetDevice());

	// TODO : error check
	return commandList;
}

RenderPass* GraphicsWebGPU::CreateRenderPass(Texture** textures, int32_t textureCount, Texture* depthTexture)
{
	assert(textures != nullptr);
	if (textures == nullptr)
		return nullptr;

	for (int32_t i = 0; i < textureCount; i++)
	{
		assert(textures[i] != nullptr);
		if (textures[i] == nullptr)
			return nullptr;
	}

	auto dt = static_cast<TextureWebGPU*>(depthTexture);

	auto renderPass = new RenderPassWebGPU();
	if (!renderPass->Initialize(textures, textureCount, dt, nullptr, nullptr))
	{
		SafeRelease(renderPass);
	}

	return renderPass;
}

RenderPass*
GraphicsWebGPU::CreateRenderPass(Texture* texture, Texture* resolvedTexture, Texture* depthTexture, Texture* resolvedDepthTexture)
{
	if (texture == nullptr)
		return nullptr;

	auto dt = static_cast<const TextureWebGPU*>(depthTexture);
	auto rt = static_cast<const TextureWebGPU*>(resolvedTexture);
	auto rdt = static_cast<const TextureWebGPU*>(resolvedDepthTexture);

	auto renderPass = new RenderPassWebGPU();
	if (!renderPass->Initialize((&texture), 1, (TextureWebGPU*)dt, (TextureWebGPU*)rt, (TextureWebGPU*)rdt))
	{
		SafeRelease(renderPass);
	}

	return renderPass;
}

Texture* GraphicsWebGPU::CreateTexture(uint64_t id) { throw "Not implemented"; }

Texture* GraphicsWebGPU::CreateTexture(const TextureParameter& parameter)
{
	auto obj = new TextureWebGPU();
	if (!obj->Initialize(GetDevice(), parameter))
	{
		SafeRelease(obj);
		return nullptr;
	}
	return obj;
}

Texture* GraphicsWebGPU::CreateTexture(const TextureInitializationParameter& parameter)
{
	TextureParameter param;
	param.Dimension = 2;
	param.Format = parameter.Format;
	param.MipLevelCount = parameter.MipMapCount;
	param.SampleCount = 1;
	param.Size = {parameter.Size.X, parameter.Size.Y, 1};
	return CreateTexture(param);
}

Texture* GraphicsWebGPU::CreateRenderTexture(const RenderTextureInitializationParameter& parameter)
{
	TextureParameter param;
	param.Dimension = 2;
	param.Format = parameter.Format;
	param.MipLevelCount = 1;
	param.SampleCount = parameter.SamplingCount;
	param.Size = {parameter.Size.X, parameter.Size.Y, 1};
	param.Usage = TextureUsageType::RenderTarget;
	return CreateTexture(param);
}

Texture* GraphicsWebGPU::CreateDepthTexture(const DepthTextureInitializationParameter& parameter)
{
	auto format = TextureFormatType::D32;
	if (parameter.Mode == DepthTextureMode::DepthStencil)
	{
		format = TextureFormatType::D24S8;
	}

	TextureParameter param;
	param.Dimension = 2;
	param.Format = format;
	param.MipLevelCount = 1;
	param.SampleCount = parameter.SamplingCount;
	param.Size = {parameter.Size.X, parameter.Size.Y, 1};
	return CreateTexture(param);
}

std::vector<uint8_t> GraphicsWebGPU::CaptureRenderTarget(Texture* renderTarget) { throw "Not implemented"; }

RenderPassPipelineState* GraphicsWebGPU::CreateRenderPassPipelineState(RenderPass* renderPass)
{
	return CreateRenderPassPipelineState(renderPass->GetKey());
}

RenderPassPipelineState* GraphicsWebGPU::CreateRenderPassPipelineState(const RenderPassPipelineStateKey& key)
{
	// already?
	{
		auto it = renderPassPipelineStates_.find(key);

		if (it != renderPassPipelineStates_.end())
		{
			auto ret = it->second;

			if (ret != nullptr)
			{
				auto ptr = ret.get();
				SafeAddRef(ptr);
				return ptr;
			}
		}
	}

	std::shared_ptr<RenderPassPipelineStateWebGPU> ret = LLGI::CreateSharedPtr<>(new RenderPassPipelineStateWebGPU());
	ret->SetKey(key);

	renderPassPipelineStates_[key] = ret;

	{
		auto ptr = ret.get();
		SafeAddRef(ptr);
		return ptr;
	}
}

} // namespace LLGI