#include "LLGI.G3.ConstantBufferMetal.h"
#include "LLGI.G3.Metal_Impl.h"

#import <MetalKit/MetalKit.h>

namespace LLGI {
namespace G3 {
    
ConstantBufferMetal::ConstantBufferMetal()
{
    impl = new Buffer_Impl();
}
    
ConstantBufferMetal::~ConstantBufferMetal()
{
    SafeDelete(impl);
}
    
bool ConstantBufferMetal::Initialize(Graphics* graphics, int32_t size)
{
    auto graphics_ = (Graphics_Impl*)graphics;
    return impl->Initialize(graphics_, size);
}
 
void* ConstantBufferMetal::Lock()
{
    throw "Not inplemented";
}
    
void* ConstantBufferMetal::Lock(int32_t offset, int32_t size)
{
    throw "Not inplemented";
}
    
void ConstantBufferMetal::Unlock()
{
    throw "Not inplemented";
}
    
int32_t ConstantBufferMetal::GetSize()
{
    throw "Not inplemented";
}

    
}
}
