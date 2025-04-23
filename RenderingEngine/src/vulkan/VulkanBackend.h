#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vk_enum_string_helper.h>

#include <vma/vk_mem_alloc.h>

#include "../Vector2.h"
#include "../Vector2_SIMD.h"
#include "../SimpleNeuralNetwork.h"
#include "../NeuralNetwork.h"
#include "../Environment.h"
#include "../DQNAgent.h"
#include "../LossFunction.h"
#include "../RecurrentNeuralNetwork.h"

#include "VulkanImage.h"
#include "VulkanDescriptor.h"
#include "VulkanPipeline.h"

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

		VkDevice device;
		VmaAllocator allocator;

		static const size_t kBufferSize = 80;
		const char* kGrayScaleTable = ".:-=+*#%@";
		static const size_t kGrayScaleTableSize = sizeof(kGrayScaleTable) / sizeof(char);

		void TrainDQN();
		void AccumulateWaveToHeightField(const double x, const double waveLength, const double maxHeight, std::array<double, kBufferSize>& heightField);
		void UpdateWave(const double timeInterval, double& x, double& speed);
		void DrawFluid(const std::array<double, kBufferSize>& heightField) const;
		void Fluid();
		void Draw();

		VulkanBackend() = delete;
		VulkanBackend(HINSTANCE hInstance, HWND hwnd);
		~VulkanBackend();

	private:

		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;

		VkSwapchainKHR swapchain;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		std::vector<VkCommandPool> swapchainCmdPools;
		std::vector<VkCommandBuffer> swapchainCmdBuffers;
		std::vector<VkFence> swapchainFences;
		std::vector<VkSemaphore> swapchainSemaphores;
		std::vector<VkSemaphore> renderSemaphores;
		uint32_t currentFrame;
		uint32_t frameNumber;

		VkCommandPool immCmdPool;
		VkCommandBuffer immCmdBuffer;
		VkFence immFence;

		std::unique_ptr<VulkanImage> offscreenFramebuffer;
		std::unique_ptr<VulkanDescriptor> computeDescriptor;
		std::unique_ptr<VulkanPipeline> computePipeline;

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
		void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	};
}