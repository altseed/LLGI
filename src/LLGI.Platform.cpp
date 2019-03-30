#include "LLGI.Platform.h"

namespace LLGI
{

bool Platform::NewFrame() { return false; }

void Platform::Present() {}

Graphics* Platform::CreateGraphics() { return nullptr; }

} // namespace LLGI