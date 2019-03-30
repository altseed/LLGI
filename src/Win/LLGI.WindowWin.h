#pragma once

#include "../LLGI.Base.h"
#include <Windows.h>
#include <string>

namespace LLGI
{

class WindowWin
{
private:
	HWND hwnd = nullptr;
	HINSTANCE hInstance = nullptr;
	std::string title_;

public:

	WindowWin() = default;

	virtual ~WindowWin() = default;

	bool Initialize(const char* title, const Vec2I& windowSize);

	bool DoEvent();

	void Terminate();

	HWND GetHandle() const { return hwnd; }
	HINSTANCE GetInstance() const { return hInstance; }
};

} // namespace LLGI