#include "LLGI.GraphicsDX12.h"
#include "LLGI.RenderPassDX12.h"

namespace LLGI
{

RenderPassDX12::RenderPassDX12(GraphicsDX12* graphics, bool isStrongRef) : graphics_(graphics), isStrongRef_(isStrongRef)
{
	if (isStrongRef_)
	{
		SafeAddRef(graphics_);
	}
}

RenderPassDX12 ::~RenderPassDX12()
{
	if (isStrongRef_)
	{
		SafeRelease(graphics_);
	}
}

RenderPassPipelineState* RenderPassDX12::CreateRenderPassPipelineState()
{
	auto ret = renderPassPipelineState.get();
	SafeAddRef(ret);
	return ret;
}

} // namespace LLGI

