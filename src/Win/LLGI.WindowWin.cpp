#include "LLGI.WindowWin.h"

namespace LLGI
{

#ifdef _WIN32
LRESULT LLGI_WndProc_Win(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

bool WindowWin::Initialize(const char* title, const Vec2I& windowSize) {

	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = (WNDPROC)LLGI_WndProc_Win;
	wcex.lpszClassName = title;
	wcex.hInstance = GetModuleHandle(NULL);
	hInstance = wcex.hInstance;
	RegisterClassExA(&wcex);

	auto wflags = WS_OVERLAPPEDWINDOW;
	RECT rect;
	rect.left = rect.top = 0;
	rect.right = windowSize.X;
	rect.bottom = windowSize.Y;
	::AdjustWindowRect(&rect, wflags, false);

	hwnd = CreateWindowA(title, title, wflags, 100, 100, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wcex.hInstance, NULL);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	title_ = title;

	// TODO : check many things
	return true;
}

bool WindowWin::DoEvent() {

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				return false;

			continue;
		}
		else
		{
			break;
		}
	}

	return true;
}

void WindowWin::Terminate() {

#ifdef _WIN32
	DestroyWindow(hwnd);
	UnregisterClassA(title_.c_str(), GetModuleHandle(NULL));
#endif
}

} // namespace LLGI