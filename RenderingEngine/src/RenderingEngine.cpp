#include "pch.h"

#include "RenderingEngine.h"

namespace Engine
{
	Random* Random::instance = nullptr;

	RenderingEngine::RenderingEngine(HINSTANCE hInstance, HWND hwnd)
	{
		// Create the Vulkan backend for rendering.
		vulkanBackend = std::make_unique<VulkanBackend>(hInstance, hwnd);

		// Create the DX12 backend for rendering.
		dx12Backend = std::make_unique<DX12Backend>(hInstance, hwnd);

		// Determine window dimensions.
		imageWidth = SCREEN_WIDTH;
		imageHeight = int(imageWidth / ASPECT_RATIO);
		imageHeight = (imageHeight < 1) ? 1 : imageHeight;
	}

	RenderingEngine::~RenderingEngine()
	{
		vulkanBackend.reset();
		dx12Backend.reset();
	}

	void RenderingEngine::Update()
	{
		switch (selectedBackend)
		{
		case RenderingBackend::VULKAN:
			vulkanBackend->Draw();
			break;
		case RenderingBackend::DX12:
			dx12Backend->Draw();
			break;
		}
	}
}