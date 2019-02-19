
#pragma once

#include "../../LLGI.Base.h"

#ifdef _WIN32
#define VK_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace LLGI
{
namespace G3
{

class GraphicsVulkan;
}
} // namespace LLGI