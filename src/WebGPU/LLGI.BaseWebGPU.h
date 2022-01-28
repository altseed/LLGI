
#pragma once

#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include "../LLGI.Base.h"

namespace LLGI
{

wgpu::CompareFunction Convert(CompareFuncType type);

wgpu::CullMode Convert(CullingMode mode);

wgpu::VertexFormat Convert(VertexLayoutFormat format);

wgpu::StencilOperation Convert(StencilOperatorType type);

wgpu::TextureFormat ConvertFormat(TextureFormatType format);

TextureFormatType ConvertFormat(wgpu::TextureFormat format);

} // namespace std
