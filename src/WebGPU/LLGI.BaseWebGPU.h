
#pragma once

#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include "../LLGI.Base.h"

namespace LLGI
{

wgpu::TextureFormat ConvertFormat(TextureFormatType format);

TextureFormatType ConvertFormat(wgpu::TextureFormat format);

} // namespace std
