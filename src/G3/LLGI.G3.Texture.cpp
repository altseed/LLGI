#include "LLGI.G3.Texture.h"

namespace LLGI
{
namespace G3
{

void* Texture::Lock()
{
	return nullptr;
}

void Texture::Unlock()
{
}

bool Texture::IsRenderTarget() const
{
	return false;
}

bool Texture::IsDepthTexture() const
{
	return false;
}

}
}