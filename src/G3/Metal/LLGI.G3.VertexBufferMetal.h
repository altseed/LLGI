#pragma once

#include "../LLGI.G3.VertexBuffer.h"

namespace LLGI
{
namespace G3
{

struct Buffer_Impl;

class VertexBufferMetal : public VertexBuffer
{
private:
	Buffer_Impl* impl = nullptr;

public:
	VertexBufferMetal();
	virtual ~VertexBufferMetal();

	bool Initialize(Graphics* graphics, int32_t size);

	void* Lock() override;
	void* Lock(int32_t offset, int32_t size) override;
	void Unlock() override;
	int32_t GetSize() override;
};

} // namespace G3
} // namespace LLGI
