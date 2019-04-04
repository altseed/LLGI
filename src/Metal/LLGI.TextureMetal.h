
#pragma once

#include "../LLGI.Texture.h"

namespace LLGI
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

    
} // namespace LLGI
