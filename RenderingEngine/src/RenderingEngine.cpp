#include "pch.h"

#include "RenderingEngine.h"
#include "Vector3.h"
#include "Ray.h"
#include "Sphere.h"
#include "HittableList.h"
#include "Interval.h"
#include "Random.h"

namespace Engine
{
	const Interval Interval::empty = Interval(+INF, -INF);
	const Interval Interval::universe = Interval(-INF, INF);

	Random* Random::instance = nullptr;

	RenderingEngine::RenderingEngine()
	{
		// Initialize SDL2.
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			throw std::runtime_error(SDL_GetError());
		}

		// Determine window dimensions.
		imageWidth = SCREEN_WIDTH;
		imageHeight = int(imageWidth / ASPECT_RATIO);
		imageHeight = (imageHeight < 1) ? 1 : imageHeight;
		SCREEN_HEIGHT = imageHeight;

		// Create window surface.
		pWindow = SDL_CreateWindow("Rendering Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (!pWindow)
		{
			SDL_Quit();

			throw std::runtime_error(SDL_GetError());
		}

		// Get render driver info.
		int numDrivers = SDL_GetNumRenderDrivers();
		SDL_RendererInfo info;
		for (int i = 0; i < numDrivers; i++)
		{
			SDL_GetRenderDriverInfo(i, &info);
			std::cout << "Renderer " << i << ": " << info.name << std::endl;
		}

		// Create rendering context.
		pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
		if (!pRenderer)
		{
			SDL_DestroyWindow(pWindow);
			SDL_Quit();

			throw std::runtime_error(SDL_GetError());
		}
	}

	RenderingEngine::~RenderingEngine()
	{
		SDL_DestroyRenderer(pRenderer);
		SDL_DestroyWindow(pWindow);
		SDL_Quit();
	}

	void RenderingEngine::Run()
	{
		// Camera.
		std::unique_ptr<Camera> camera = std::make_unique<Camera>(SCREEN_WIDTH, 16.0 / 9.0);

		// Geometry.
		HittableList world;
		world.Add(std::make_shared<Sphere>(Vector3(0, 0, -1), 0.5));
		world.Add(std::make_shared<Sphere>(Vector3(0, -100.5, -1), 100));

		// Accumulation buffer.
		Vector3* accumulationBuffer = new Vector3[SCREEN_WIDTH * SCREEN_HEIGHT];

		// Output image.
		uint32_t* pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
		SDL_Texture* texture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Timer.
		auto lastFrameTime = std::chrono::high_resolution_clock::now();

		long frameCounter = 0;
		int counter = 0;
		bool running = true;
		SDL_Event event;

		// MAIN LOOP.
		while (running)
		{
			frameCounter++;

			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}

				if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
				}
			}

			for (int y = 0; y < SCREEN_HEIGHT; y++)
			{
				for (int x = 0; x < SCREEN_WIDTH; x++)
				{
					const Ray ray = camera->GetJitteredRay(x, y);

					Vector3 color;
					HitRecord record;
					if (world.Hit(ray, Interval(0, INF), record))
					{
						color = 0.5 * (record.N + Vector3(1, 1, 1));
					}
					else
					{
						const double t = 0.5 * (ray.direction.y() + 1.0);
						color = Vector3::Lerp(Vector3(1.0, 1.0, 1.0), Vector3(0.5, 0.7, 1.0), t);
					}

					const int index = y * SCREEN_WIDTH + x;

					// Read previous color from accumulation buffer.
					const Vector3 previousColor = accumulationBuffer[index];

					// Add the current color to the accumulated color and update the buffer.
					const Vector3 accumulatedColor = previousColor + color;
					accumulationBuffer[index] = accumulatedColor;

					color = accumulatedColor / frameCounter;

					// Convert 
					static const Interval intensity(0.000, 0.999);
					const uint8_t r = 256 * intensity.Clamp(color.x());
					const uint8_t g = 256 * intensity.Clamp(color.y());
					const uint8_t b = 256 * intensity.Clamp(color.z());
					const uint8_t a = 255;

					pixels[y * SCREEN_WIDTH + x] = (a << 24) | (b << 16) | (g << 8) | r;
				}
			}

			SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));

			SDL_RenderClear(pRenderer);						// Clear the screen
			SDL_RenderCopy(pRenderer, texture, NULL, NULL); // Draw texture image to screen
			SDL_RenderPresent(pRenderer);					// Present frame

			// Frame time.
			auto currentFrameTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> deltaTime = currentFrameTime - lastFrameTime;
			lastFrameTime = currentFrameTime;

			const double frameTime_ms = deltaTime.count() * 1000.0f;
			std::cout << frameTime_ms << " ms.\n";
		}

		// Cleanup.
		delete[](pixels);
		delete[](accumulationBuffer);
		SDL_DestroyTexture(texture);
	}

	void RenderingEngine::Render()
	{

	}
}