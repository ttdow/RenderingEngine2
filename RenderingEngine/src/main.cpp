#include "pch.h"

#include "RenderingEngine.h"

// Windows message handler.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);		 // Posts a WM_QUIT message to end the main loop.
		return EXIT_SUCCESS;
	case WM_SIZING:
		break;
	case WM_SIZE:
		//Resize(hwnd);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);	 // ESC exits app.
			return EXIT_SUCCESS;
		}
		break;
	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam); // Default message handling.
	}

	return DefWindowProc(hwnd, msg, wParam, lParam); // Fallback.
}

HWND MakeWindow(HINSTANCE hInstance, int width, int height)
{
	const wchar_t CLASS_NAME[] = L"RenderingEngine";

	WNDCLASS wc = {};
	wc.lpfnWndProc = &WndProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, style, FALSE);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, L"Rendering Engine",
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
#if defined _DEBUG

	// Manually create a console window for stdout/stderr/stdin.
	AllocConsole();
	FILE* _ = freopen("CONOUT$", "w", stdout);
	_ = freopen("CONOUT$", "w", stderr);
	_ = freopen("CONIN$", "r", stdin);

#endif

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	HWND hwnd = MakeWindow(hInstance, SCREEN_WIDTH, SCREEN_HEIGHT);

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

			// Rendering system update.
			engine->Update();
		}

		// Destroy rendering engine.
		engine.reset();
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR::" << e.what() << std::endl;

#if defined _DEBUG

		std::cout << "Press ENTER to contine...";
		std::cin.get();

#endif

		return EXIT_FAILURE;
	}

#if defined _DEBUG

	std::cout << "Press ENTER to contine...";
	std::cin.get();

#endif

	return EXIT_SUCCESS;
}