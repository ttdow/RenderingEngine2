#pragma once

#include <SDL2/SDL.h>

#include "Camera.h"

namespace Engine
{
	class RenderingEngine
	{
	public:

		RenderingEngine();
		~RenderingEngine();

		void Run();

	private:

		const int SCREEN_WIDTH = 800;
		const double ASPECT_RATIO = 16.0 / 9.0;
		int SCREEN_HEIGHT;
		int imageWidth, imageHeight;

		SDL_Window* pWindow = nullptr;
		SDL_Renderer* pRenderer = nullptr;

		void Render();
	};
}