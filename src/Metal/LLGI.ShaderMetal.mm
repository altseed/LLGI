#include "LLGI.ShaderMetal.h"
#include "LLGI.Metal_Impl.h"
#import <MetalKit/MetalKit.h>
#include <iostream>

namespace LLGI
{

    
Shader_Impl::Shader_Impl()
{
}
    
Shader_Impl::~Shader_Impl()
    {
        if(library != nullptr)
        {
            [library release];
            library = nullptr;
        }
    }
    
    bool Shader_Impl::Initialize(Graphics_Impl* graphics, void* data, int size)
    {
        auto device = graphics->device;
        
        NSError* libraryError = nil;
        id<MTLLibrary> lib = [device newLibraryWithData:(dispatch_data_t)data error:&libraryError];
        
        if (libraryError)
        {
            std::cout << libraryError.localizedDescription.UTF8String << std::endl;
            return false;
        }
        
        this->library = lib;
        
        return true;
    }

    
}

