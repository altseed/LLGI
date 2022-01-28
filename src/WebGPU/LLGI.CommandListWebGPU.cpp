#include "LLGI.CommandListWebGPU.h"

namespace LLGI
{

void CommandListWebGPU::SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	renderPassEncorder_.SetScissorRect(x, y, width, height);
}

} // namespace LLGI