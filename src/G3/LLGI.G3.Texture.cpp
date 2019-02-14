#include "LLGI.G3.Texture.h"

namespace LLGI
{
namespace G3
{

void* Texture::Lock() { return nullptr; }

void Texture::Unlock() {}

Vec2I Texture::GetSizeAs2D() { return Vec2I(); }

bool Texture::IsRenderTexture() const { return false; }

bool Texture::IsDepthTexture() const { return false; }

} // namespace G3
} // namespace LLGI