
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{

Platform* CreatePlatform(DeviceType platformDeviceType);

class Platform : public ReferenceObject
{
private:
public:
	Platform() = default;
	virtual ~Platform() = default;

	virtual bool NewFrame();
	virtual void Present();
	virtual Graphics* CreateGraphics();
	virtual DeviceType GetDeviceType() const { return DeviceType::Default; }

	virtual void SetWindowSize(const Vec2I& windowSize);

	/**
		@brief get render pass of screen to show on a display.
		@note
		Don't release and addref it.
		Don't use it for the many purposes, please input Clear or SetRenderPass immediately.
	*/
	virtual RenderPass* GetCurrentScreen(const Color8& clearColor = Color8(), bool isColorCleared = false, bool isDepthCleared = false);
};

} // namespace LLGI