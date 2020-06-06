
#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace LLGI
{

static const int RenderTargetMax = 8;
static const int VertexLayoutMax = 16;

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
	R32G32B32A32_FLOAT,
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

	bool operator==(const Vec2I& o) const { return X == o.X && Y == o.Y; }
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

struct ColorF
{
	float R;
	float G;
	float B;
	float A;

	ColorF() : R(1.0f), G(1.0f), B(1.0f), A(1.0f) {}

	ColorF(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
};

enum class TextureFormatType
{
	R8G8B8A8_UNORM = 0,
	R16G16B16A16_FLOAT = 11,
	R32G32B32A32_FLOAT = 1,
	R8G8B8A8_UNORM_SRGB = 2,
	R16G16_FLOAT = 3,
	R8_UNORM = 4,

	BC1 = 5,
	BC2 = 6,
	BC3 = 7,
	BC1_SRGB = 8,
	BC2_SRGB = 9,
	BC3_SRGB = 10,

	//! for internal
	B8G8R8A8_UNORM = 254,
	Uknown = 255,
};

enum class TextureType
{
	Screen,
	Color,
	Depth,
	Render,
	Cube,
	Unknown,
};

struct DataStructure
{
	const void* Data;
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
		auto p_ = (p);
		SafeRelease(p_);
	}
};

template <typename T> static std::shared_ptr<T> CreateSharedPtr(T* p, bool addRef = false)
{
	if (addRef)
	{
		SafeAddRef(p);
	}

	return std::shared_ptr<T>(p, ReferenceDeleter<T>());
}

template <typename T> inline std::unique_ptr<T, ReferenceDeleter<T>> CreateUniqueReference(T* ptr, bool addRef = false)
{
	if (ptr == nullptr)
		return std::unique_ptr<T, ReferenceDeleter<T>>(nullptr);

	if (addRef)
	{
		ptr->AddRef();
	}

	return std::unique_ptr<T, ReferenceDeleter<T>>(ptr);
}

template <typename T> using unique_ref = std::unique_ptr<T, ReferenceDeleter<T>>;

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

enum class LogType
{
	Info,
	Warning,
	Error,
	Debug,
};

void SetLogger(const std::function<void(LogType, const char*)>& logger);

void Log(LogType logType, const char* message);

inline size_t GetAlignedSize(size_t size, size_t alignment) { return (size + (alignment - 1)) & ~(alignment - 1); }

inline int32_t GetTextureMemorySize(TextureFormatType format, Vec2I size)
{
	switch (format)
	{
	case TextureFormatType::R8G8B8A8_UNORM:
		return size.X * size.Y * 4;
	case TextureFormatType::R16G16B16A16_FLOAT:
		return size.X * size.Y * 8;
	case TextureFormatType::R32G32B32A32_FLOAT:
		return size.X * size.Y * 16;
	case TextureFormatType::R8G8B8A8_UNORM_SRGB:
		return size.X * size.Y * 4;
	case TextureFormatType::R16G16_FLOAT:
		return size.X * size.Y * 4;
	case TextureFormatType::R8_UNORM:
		return size.X * size.Y * 1;
	default:
		assert(0);
	}
	return 0;
}

/**
	@brief	window abstraction class
*/
class Window
{
public:
	Window() = default;

	virtual ~Window() = default;

	/**
		@brief	called when platform calls newframe, if it need to exit, return false
	*/
	virtual bool OnNewFrame() = 0;

	/**
		@brief return native window ptr
	*/
	virtual void* GetNativePtr(int32_t index) = 0;

	/**
		@brief	return current window size
	*/
	virtual Vec2I GetWindowSize() const = 0;

	/**
		@brief  return current frame buffer  size
	*/
	virtual Vec2I GetFrameBufferSize() const { return GetWindowSize(); }
};

} // namespace LLGI
