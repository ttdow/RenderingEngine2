#pragma once

#include <Windows.h>

#include "Camera.h"
#include "vulkan/VulkanBackend.h"
#include "dx12/DX12Backend.h"

namespace Engine
{
	enum RenderingBackend
	{
		VULKAN = 0,
		DX12 = 1
	};

	class RenderingEngine
	{
	public:

		RenderingEngine() = delete;
		RenderingEngine(HINSTANCE hInstance, HWND hwnd);
		~RenderingEngine();

		void Update();

	private:

		std::unique_ptr<VulkanBackend> vulkanBackend;
		std::unique_ptr<DX12Backend> dx12Backend;

		RenderingBackend selectedBackend = RenderingBackend::DX12;
		int imageWidth, imageHeight;
	};
}