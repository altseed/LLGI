
#pragma once

#include "../LLGI.G3.VertexBuffer.h"
#include "LLGI.G3.BaseVulkan.h"
#include "LLGI.G3.GraphicsVulkan.h"

namespace LLGI
{
namespace G3
{

class VertexBufferVulkan : public VertexBuffer
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::unique_ptr<Buffer> cpuBuf;
	std::unique_ptr<Buffer> gpuBuf;
	void* data = nullptr;
	int32_t memSize = 0;

public:
	bool Initialize(GraphicsVulkan* graphics, int32_t size);

	VertexBufferVulkan();
	virtual ~VertexBufferVulkan();

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;
	int32_t GetSize() override;
};

} // namespace G3
} // namespace LLGI
