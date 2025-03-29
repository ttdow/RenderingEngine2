#include "pch.h"

//#include "Windows.h"

#include "RenderingEngine.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateSimpleWindow(HINSTANCE hInstance, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"MinimalVulkanWindow";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	DWORD style = WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, style, FALSE);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, L"Test Window",
		style, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, hInstance, nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	return hwnd;
}

/**
 * @brief Application entry point.
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine
 * @param nCmdShow
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Manually create a console window for stdout/stderr/stdin.
	AllocConsole();
	FILE* _ = freopen("CONOUT$", "w", stdout);
	_ = freopen("CONOUT$", "w", stderr);
	_ = freopen("CONIN$", "r", stdin);

	HWND hwnd = CreateSimpleWindow(hInstance, 800, 600);

	try
	{
		// Create rendering engine.
		std::unique_ptr<Engine::RenderingEngine> engine = std::make_unique<Engine::RenderingEngine>(hInstance, hwnd);

		// Main loop.
		MSG msg = {};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// Add rendering or event handling here.
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR::" << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}