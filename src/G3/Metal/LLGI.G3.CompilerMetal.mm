#include "LLGI.G3.CompilerMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{
namespace G3
{

void CompilerMetal::Initialize() {}

void CompilerMetal::Compile(CompilerResult& result, const char* code, ShaderStageType shaderStage)
{
	NSString* code_ = [[NSString alloc] initWithUTF8String:code];

	id<MTLDevice> device = MTLCreateSystemDefaultDevice();

	NSError* libraryError = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:code_ options:NULL error:&libraryError];
	if (libraryError)
	{
		result.Message = libraryError.localizedDescription.UTF8String;
		return;
	}
}

}
}
