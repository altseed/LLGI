
#pragma once

#include "../LLGI.Texture.h"
#include "LLGI.GraphicsMetal.h"

namespace LLGI
{

struct Texture_Impl;

class TextureMetal : public Texture
{
private:
    ReferenceObject* owner_ = nullptr;
	Texture_Impl* impl = nullptr;
	std::vector<uint8_t> data;
    bool isRenderTexture_ = false;
    bool isDepthTexture_ = false;
    
public:
	TextureMetal();
	virtual ~TextureMetal();

	bool Initialize(id<MTLDevice> device, ReferenceObject* owner, Vec2I size, bool isRenderTexture, bool isDepthTexture);
	bool Initialize(GraphicsMetal* owner, const RenderTextureInitializationParameter& parameter);
	bool Initialize();
	void Reset(id<MTLTexture> nativeTexture);
	void* Lock() override;
	void Unlock() override;
	Vec2I GetSizeAs2D() override;
	bool IsRenderTexture() const override;
	bool IsDepthTexture() const override;

	Texture_Impl* GetImpl() const;
};

} // namespace LLGI
