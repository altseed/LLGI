
#pragma once

#include "../LLGI.ConstantBuffer.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

class ConstantBufferVulkan : public ConstantBuffer
{
private:
	std::shared_ptr<GraphicsVulkan> graphics_;
	std::unique_ptr<Buffer> buffer;
	int memSize = 0;
	void* data = nullptr;

public:
	ConstantBufferVulkan();
	virtual ~ConstantBufferVulkan();

	bool Initialize(GraphicsVulkan* graphics, int32_t size);

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;
	int32_t GetSize() override;
};

} // namespace LLGI
