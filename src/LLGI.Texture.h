
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

class Texture : public ReferenceObject
{
private:
public:
	Texture() = default;
	virtual ~Texture() = default;

	/**
		@brief	get an internal buffer of image
		@note
		CommandList::UpdateTexture must be called in some platforms.
	*/
	virtual void* GetBuffer() const;
	
	virtual void* Lock();
	virtual void Unlock();
	virtual Vec2I GetSizeAs2D();
	virtual bool IsRenderTexture() const;
	virtual bool IsDepthTexture() const;
};

} // namespace LLGI