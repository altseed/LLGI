
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class Texture : public ReferenceObject
{
private:
protected:
	TextureType type_ = TextureType::Unknown;
	TextureFormatType format_ = TextureFormatType::Unknown;
	int32_t samplingCount_ = 1;

public:
	Texture() = default;
	~Texture() override = default;

	/*[[deprecated("use CommandList::SetImageData2D.")]]*/ virtual void* Lock();

	/*[[deprecated("use CommandList::SetImageData2D.")]]*/ virtual void Unlock();

	virtual Vec2I GetSizeAs2D() const;
	[[deprecated("use GetType.")]] virtual bool IsRenderTexture() const;
	[[deprecated("use GetType.")]] virtual bool IsDepthTexture() const;

	TextureType GetType() const { return type_; }

	virtual TextureFormatType GetFormat() const;

	int32_t GetSamplingCount() const { return samplingCount_; }
};

} // namespace LLGI
