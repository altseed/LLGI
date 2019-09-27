
#pragma once

#include "../LLGI.Platform.h"

namespace LLGI
{
    
struct PlatformMetal_Impl;

class TextureMetal;
class RenderPassMetal;
    
class PlatformMetal : public Platform
{
	PlatformMetal_Impl* impl = nullptr;
    
    TextureMetal* renderTexture_ = nullptr;
    RenderPassMetal* renderPass_ = nullptr;
    
public:
	PlatformMetal();
	~PlatformMetal();
	bool NewFrame() override;
	void Present() override;
	Graphics* CreateGraphics() override;
    
    RenderPass* GetCurrentScreen(const Color8& clearColor, bool isColorCleared, bool isDepthCleared) override;
    
    DeviceType GetDeviceType() const override { return DeviceType::Metal; }
};

} // namespace LLGI
