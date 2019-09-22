#pragma once

#include "../LLGI.Base.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <locale.h>
#include <string>

namespace LLGI
{

class WindowLinux
{
private:
	Display*	display_ = nullptr;
	Window	window_;
	GC	gc_;
	bool	closed_ = false;
	Atom	wm_delete_window_;
	std::string title_;

public:

	WindowLinux() = default;

	virtual ~WindowLinux() = default;

	bool Initialize(const char* title, const Vec2I& windowSize);

	bool DoEvent();

	void Terminate();

	Display*& GetDisplay() { return display_; }
	Window&	GetWindow() { return window_; }
};

} // namespace LLGI