#include "LLGI.TextureMetal.h"

namespace LLGI
{


void* TextureMetal::Lock()
{
	throw "Not inplemented";
	return nullptr;
}

void TextureMetal::Unlock() { throw "Not inplemented"; }

Vec2I TextureMetal::GetSizeAs2D()
{
	throw "Not inplemented";
	return Vec2I();
}

bool TextureMetal::IsRenderTexture() const
{
	throw "Not inplemented";
	return false;
}

bool TextureMetal::IsDepthTexture() const
{
	throw "Not inplemented";
	return false;
}

}
