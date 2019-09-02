#include "LLGI.Platform.h"

namespace LLGI
{

bool Platform::NewFrame() { return false; }

void Platform::Present() {}

Graphics* Platform::CreateGraphics() { return nullptr; }

void Platform::SetWindowSize(const Vec2I& windowSize)
{
	assert(0); // TODO: Not implemented.
}

RenderPass* Platform::GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared)
{
	assert(0); // TODO: Not implemented.
	return nullptr;
}

} // namespace LLGI