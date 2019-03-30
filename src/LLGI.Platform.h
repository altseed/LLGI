
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
};

} // namespace LLGI