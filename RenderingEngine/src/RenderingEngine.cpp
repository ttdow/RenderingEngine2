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
	Random* Random::instance = nullptr;

	RenderingEngine::RenderingEngine(HINSTANCE hInstance, HWND hwnd)
	{
		// Create the Vulkan backend for rendering.
		vulkanBackend = std::make_unique<VulkanBackend>(hInstance, hwnd);

		// Determine window dimensions.
		imageWidth = SCREEN_WIDTH;
		imageHeight = int(imageWidth / ASPECT_RATIO);
		imageHeight = (imageHeight < 1) ? 1 : imageHeight;
		SCREEN_HEIGHT = imageHeight;
	}

	RenderingEngine::~RenderingEngine()
	{
		vulkanBackend.reset(); // Delete object and set the pointer to nullptr.
	}

	void RenderingEngine::Run()
	{
		/*
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
		*/
	}

	void RenderingEngine::Render()
	{

	}
}