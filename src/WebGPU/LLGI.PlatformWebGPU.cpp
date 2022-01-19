#include "LLGI.PlatformWebGPU.h"
#include "LLGI.GraphicsWebGPU.h"

namespace LLGI
{

int PlatformWebGPU::GetCurrentFrameIndex() const { return 0; }

int PlatformWebGPU::GetMaxFrameCount() const { return 1; }

bool PlatformWebGPU::NewFrame() { return false; }

void PlatformWebGPU::Present() {}

Graphics* PlatformWebGPU::CreateGraphics()
{
	auto ret = new GraphicsWebGPU(device_);
	return ret;
}

} // namespace LLGI