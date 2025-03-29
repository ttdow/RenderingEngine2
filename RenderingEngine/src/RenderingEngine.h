#pragma once

#include <Windows.h>

#include "Camera.h"
#include "VulkanBackend.h"

namespace Engine
{
	class RenderingEngine
	{
	public:

		RenderingEngine() = delete;
		RenderingEngine(HINSTANCE hInstance, HWND hwnd);
		~RenderingEngine();

		void Run();

	private:

		std::unique_ptr<VulkanBackend> vulkanBackend;

		const int SCREEN_WIDTH = 800;
		const double ASPECT_RATIO = 16.0 / 9.0;
		int SCREEN_HEIGHT;
		int imageWidth, imageHeight;

		void Render();
	};
}