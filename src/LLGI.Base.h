
#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <array>
#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace LLGI
{

enum class DeviceType
{
	Default,
	DirectX12,
	Metal,
	Vulkan,
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

enum class BlendEquationType
{
	Add,
	Sub,
	ReverseSub,
	Min,
	Max,
};

enum class BlendFuncType
{
	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	DstColor,
	OneMinusDstColor,
};

enum class VertexLayoutFormat
{
	R32G32B32_FLOAT,
	R8G8B8A8_UNORM,
	R8G8B8A8_UINT,
	R32G32_FLOAT,
};

enum class TopologyType
{
	Triangle,
	Line,
};

enum class TextureWrapMode
{
	Clamp,
	Repeat,
};

enum class TextureMinMagFilter
{
	Nearest,
	Linear,
};

enum class DepthFuncType
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
};

enum class ConstantBufferType
{
	LongTime,  //! this constant buffer is not almost changed
	ShortTime, //! this constant buffer is disposed or rewrite by a frame. If shorttime, this constant buffer must be disposed by a frame.
};

struct Vec2I
{
	int32_t X;
	int32_t Y;

	Vec2I() : X(0), Y(0) {}

	Vec2I(int32_t x, int32_t y) : X(x), Y(y) {}
};

struct Vec2F
{
	float X;
	float Y;

	Vec2F() : X(0), Y(0) {}

	Vec2F(float x, float y) : X(x), Y(y) {}
};

struct Vec3F
{
	float X;
	float Y;
	float Z;

	Vec3F() : X(0), Y(0), Z(0) {}

	Vec3F(float x, float y, float z) : X(x), Y(y), Z(z) {}
};

struct Color8
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;

	Color8() : R(255), G(255), B(255), A(255) {}

	Color8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}
};

struct DataStructure
{
	void* Data;
	int32_t Size;
};

template <class T> void SafeAddRef(T& t)
{
	if (t != NULL)
	{
		t->AddRef();
	}
}

template <class T> void SafeRelease(T& t)
{
	if (t != NULL)
	{
		t->Release();
		t = NULL;
	}
}

template <class T> void SafeAssign(T& t, T value)
{
	SafeAddRef(value);
	SafeRelease(t);
	t = value;
}

template <class T> void SafeDelete(T& t)
{
	if (t != NULL)
	{
		delete t;
		t = NULL;
	}
}

class ReferenceObject
{
private:
	mutable std::atomic<int32_t> reference;

public:
	ReferenceObject() : reference(1) {}

	virtual ~ReferenceObject() {}

	int AddRef()
	{
		std::atomic_fetch_add_explicit(&reference, 1, std::memory_order_consume);
		return reference;
	}

	int GetRef() { return reference; }

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

template <typename T> struct ReferenceDeleter
{
	void operator()(T* p)
	{
		auto p_ = ((ReferenceObject*)p);
		SafeRelease(p_);
	}
};

template <typename T> static std::shared_ptr<T> CreateSharedPtr(T* p) { return std::shared_ptr<T>(p, ReferenceDeleter<T>()); }

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Shader;
class PipelineState;
class Texture;
class Platform;
class Graphics;
class CommandList;
class Compiler;
class RenderPass;
class RenderPassPipelineState;

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
class Compiler;
class RenderPass;

} // namespace G3

} // namespace LLGI
