
#pragma once

#include "../LLGI.Platform.h"

namespace LLGI
{

struct PlatformMetal_Impl;

class PlatformMetal : public Platform
{
	PlatformMetal_Impl* impl = nullptr;

public:
	PlatformMetal();
	~PlatformMetal();
	bool NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;
    
    DeviceType GetDeviceType() const override { return DeviceType::Metal; }
};

} // namespace LLGI
