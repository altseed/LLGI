
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <memory>
#include <atomic>
#include <array>
#include <vector>

namespace LLGI
{

enum class PlatformType
{
	Default,
};

enum class ErrorCode
{
	OK,
	Failed,
};

enum class ShaderStageType
{
	Vertex,
	Pixel,
	Max,
};

enum class CullingMode
{
	Clockwise,
	CounterClockwise,
	DoubleSide,
};

enum class VertexLayoutFormat
{
	R32G32B32_FLOAT,
	R8G8B8A8_UNORM,
	R8G8B8A8_UINT,
	R32G32_FLOAT,
};

struct Vec2I
{
	int32_t X;
	int32_t Y;
};

template <class T>
void SafeAddRef(T& t)
{
	if (t != NULL)
	{
		t->AddRef();
	}
}

template <class T>
void SafeRelease(T& t)
{
	if (t != NULL)
	{
		t->Release();
		t = NULL;
	}
}

class ReferenceObject
{
private:
	mutable std::atomic<int32_t> reference;

public:
	ReferenceObject()
		: reference(1)
	{
	}

	virtual ~ReferenceObject()
	{
	}

	int AddRef()
	{
		std::atomic_fetch_add_explicit(&reference, 1, std::memory_order_consume);
		return reference;
	}

	int GetRef()
	{
		return reference;
	}

	int Release()
	{
		assert(reference > 0);

		bool destroy = std::atomic_fetch_sub_explicit(&reference, 1, std::memory_order_consume) == 1;
		if (destroy)
		{
			delete this;
			return 0;
		}

		return reference;
	}
};

template <typename T>
struct ReferenceDeleter
{
	void operator ()(T* p)
	{
		auto p_ = ((ReferenceObject*)p);
		SafeRelease(p_);
	}
};

template <typename T>
static std::shared_ptr<T> CreateSharedPtr(T* p)
{
	return std::shared_ptr<T>(p, ReferenceDeleter<T>());
}

namespace G3
{

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Shader;
class PipelineState;
class Texture;
class Platform;
class Graphics;
class CommandList;

}

}