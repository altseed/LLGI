#include "ImGuiPlatformMetal.h"

#include "../thirdparty/imgui/imgui_impl_metal.h"
#include <Metal/LLGI.CommandListMetal.h>
#include <Metal/LLGI.GraphicsMetal.h>
#include <Metal/LLGI.Metal_Impl.h>

class ImguiPlatformMetal_Impl
{
	LLGI::GraphicsMetal* g_ = nullptr;

public:
	ImguiPlatformMetal_Impl(LLGI::Graphics* g) : g_(static_cast<LLGI::GraphicsMetal*>(g)) { ImGui_ImplMetal_Init(g_->GetImpl()->device); }

	virtual ~ImguiPlatformMetal_Impl() { ImGui_ImplMetal_Shutdown(); }

	void NewFrame() { ImGui_ImplMetal_NewFrame(); }

	void RenderDrawData(ImDrawData* draw_data, LLGI::CommandList* commandList)
	{
		auto cl = static_cast<LLGI::CommandListMetal*>(commandList);
		ImGui_ImplMetal_RenderDrawData(
			ImGui::GetDrawData(), cl->GetCommandList(), cl->GetImpl()->commandBuffer, cl->GetImpl()->renderEncoder);
	}
};

ImguiPlatformMetal::ImguiPlatformMetal(LLGI::Graphics* g)
{
	impl = new ImguiPlatformMetal_Impl(g);
}

ImguiPlatformMetal::~ImguiPlatformMetal()
{
	delete impl;
}

void ImguiPlatformMetal::NewFrame()
{
	impl->NewFrame();
}

void ImguiPlatformMetal::RenderDrawData(ImDrawData* draw_data, LLGI::CommandList* commandList) 
{
	impl->RenderDrawData(draw_data, commandList) ;
}