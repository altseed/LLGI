
#pragma once

#include "../LLGI.G3.Platform.h"

namespace LLGI {
namespace G3{

    struct PlatformMetal_Impl;
    
    class PlatformMetal
    : public Platform
    {
        PlatformMetal_Impl* impl = nullptr;
    public:
        
        PlatformMetal();
        ~PlatformMetal();
        void NewFrame() override;
        void Present() override;
        Graphics* CreateGraphics() override;
        
    };
    
}
}
