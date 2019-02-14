#pragma once

#include "../LLGI.G3.ConstantBuffer.h"

namespace LLGI
{
namespace G3
{

struct Buffer_Impl;

class ConstantBufferMetal : public ConstantBuffer
{
private:
	Buffer_Impl* impl = nullptr;

public:
	ConstantBufferMetal();
	virtual ~ConstantBufferMetal();

	bool Initialize(Graphics* graphics, int32_t size);

	void* Lock() override;

	void* Lock(int32_t offset, int32_t size) override;

	void Unlock() override;

	int32_t GetSize() override;
};

} // namespace G3
} // namespace LLGI
