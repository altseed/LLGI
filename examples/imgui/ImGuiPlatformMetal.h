
#pragma once

#include "ImGuiPlatform.h"

class ImguiPlatformMetal_Impl;

class ImguiPlatformMetal : ImguiPlatform
{
	ImguiPlatformMetal_Impl* impl = nullptr;

public:
	ImguiPlatformMetal(LLGI::Graphics* g);

	virtual ~ImguiPlatformMetal();

	void NewFrame() override;

	void RenderDrawData(ImDrawData* draw_data, LLGI::CommandList* commandList) override;
};