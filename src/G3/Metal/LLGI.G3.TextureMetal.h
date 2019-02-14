
#pragma once

#include "../LLGI.G3.Texture.h"

namespace LLGI
{
namespace G3
{

class TextureMetal : public Texture
{
private:
public:
	TextureMetal() = default;
	virtual ~TextureMetal() = default;

	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;
};

} // namespace G3
} // namespace LLGI
