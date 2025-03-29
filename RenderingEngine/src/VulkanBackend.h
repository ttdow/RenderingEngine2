#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vk_enum_string_helper.h>

#include "Vector2.h"
#include "Vector2_SIMD.h"
#include "SimpleNeuralNetwork.h"
#include "NeuralNetwork.h"
#include "Environment.h"
#include "DQNAgent.h"

namespace Engine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value() &&
				computeFamily.has_value() &&
				transferFamily.has_value() &&
				presentFamily.has_value();
		}
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanBackend
	{
	public:

		void TrainDQN();

		VulkanBackend() = delete;
		VulkanBackend(HINSTANCE hInstance, HWND hwnd);
		~VulkanBackend();

	private:

		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		VkQueue graphicsQueue;
		VkQueue computeQueue;
		VkQueue transferQueue;
		VkQueue presentQueue;

		QueueFamilyIndices queueFamilyIndices;
		SwapchainSupportDetails swapchainSupportDetails;

		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR physicalDeviceRTProperties;
		VkPhysicalDeviceProperties2 physicalDeviceProperties2;

		VkFormat imageFormat;
		VkExtent2D imageExtent;
		uint32_t framebufferCount;

		bool hardwareRaytracingSupported;

		int RateDeviceSuitability(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;
	};
}