#include "LLGI.Graphics.h"

namespace LLGI
{

//! TODO should be moved
static std::function<void(LogType, const char*)> g_logger;

void SetLogger(const std::function<void(LogType, const char*)>& logger) { g_logger = logger; }

void Log(LogType logType, const char* message)
{
	if (g_logger != nullptr)
	{
		g_logger(logType, message);
	}
}

void SingleFrameMemoryPool::NewFrame() {}

ConstantBuffer* SingleFrameMemoryPool::CreateConstantBuffer(int32_t size) { return nullptr; }

void RenderPass::SetIsColorCleared(bool isColorCleared) { isColorCleared_ = isColorCleared; }

void RenderPass::SetIsDepthCleared(bool isDepthCleared) { isDepthCleared_ = isDepthCleared; }

void RenderPass::SetClearColor(const Color8& color) { color_ = color; }

Texture* RenderPass::GetColorBuffer(int index)
{
	Log(LogType::Error, "GetColorBuffer is not implemented.");
	assert(0);
	return nullptr;
}

Graphics::~Graphics()
{
	if (disposed_ != nullptr)
	{
		disposed_();
	}
}

void Graphics::SetWindowSize(const Vec2I& windowSize) { windowSize_ = windowSize; }

void Graphics::Execute(CommandList* commandList) {}

//RenderPass* Graphics::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) { return nullptr; }

VertexBuffer* Graphics::CreateVertexBuffer(int32_t size) { return nullptr; }

IndexBuffer* Graphics::CreateIndexBuffer(int32_t stride, int32_t count) { return nullptr; }

Shader* Graphics::CreateShader(DataStructure* data, int32_t count) { return nullptr; }

PipelineState* Graphics::CreatePiplineState() { return nullptr; }

SingleFrameMemoryPool* Graphics::CreateSingleFrameMemoryPool(int32_t constantBufferPoolSize, int32_t drawingCount) { return nullptr; }

CommandList* Graphics::CreateCommandList(SingleFrameMemoryPool* memoryPool) { return nullptr; }

CommandListPool* Graphics::CreateCommandListPool(int32_t constantBufferPoolSize, int32_t drawingCount, int32_t swapbufferCount)
{
	Log(LogType::Error, "GetColorBuffer is not implemented.");
	assert(0);
	return nullptr;
}

ConstantBuffer* Graphics::CreateConstantBuffer(int32_t size) { return nullptr; }

Texture* Graphics::CreateTexture(const Vec2I& size, bool isRenderPass, bool isDepthBuffer) { return nullptr; }

Texture* Graphics::CreateTexture(uint64_t id) { return nullptr; }

RenderPassPipelineState* Graphics::CreateRenderPassPipelineState(RenderPass* renderPass) { return nullptr; }

std::vector<uint8_t> Graphics::CaptureRenderTarget(Texture* renderTarget)
{
	Log(LogType::Error, "GetColorBuffer is not implemented.");
	assert(0);
	return std::vector<uint8_t>();
}

void Graphics::SetDisposed(const std::function<void()>& disposed) { disposed_ = disposed; }

} // namespace LLGI
