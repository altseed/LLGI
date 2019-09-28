#pragma once

#include "../LLGI.Base.h"
#include <string>

namespace LLGI
{

struct WindowMac_Impl;

class WindowMac
{
private:
	std::shared_ptr<WindowMac_Impl> impl = nullptr;

public:
	WindowMac() = default;

	virtual ~WindowMac() = default;

	bool Initialize(const char* title, const Vec2I& windowSize);

	bool DoEvent();

	void Terminate();

	void* GetNSWindowAsVoidPtr();
};

} // namespace LLGI
