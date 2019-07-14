#include "LLGI.Texture.h"

namespace LLGI
{

void* Texture::GetBuffer() const { return nullptr; }

void* Texture::Lock() { return nullptr; }

void Texture::Unlock() {}

Vec2I Texture::GetSizeAs2D() { return Vec2I(); }

bool Texture::IsRenderTexture() const { return false; }

bool Texture::IsDepthTexture() const { return false; }

} // namespace LLGI