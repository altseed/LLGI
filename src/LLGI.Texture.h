
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class Texture : public ReferenceObject
{
private:
protected:
	TextureFormatType format_ = TextureFormatType::Uknown;

public:
	Texture() = default;
	virtual ~Texture() = default;

	/**
		@brief	get an internal buffer of image
		@note
		CommandList::UpdateTexture must be called in some platforms.
	*/
	virtual void* GetBuffer() const;

	[[deprecated("use CommandList::SetImageData2D.")]] virtual void* Lock();

	[[deprecated("use CommandList::SetImageData2D.")]] virtual void Unlock();

	virtual Vec2I GetSizeAs2D();
	virtual bool IsRenderTexture() const;
	virtual bool IsDepthTexture() const;

	virtual TextureFormatType GetFormat() const;
};

} // namespace LLGI