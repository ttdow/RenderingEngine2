#pragma once

#include <vulkan/vulkan.h>

namespace Engine
{
	class VulkanPipeline
	{
	public:

		VkDevice device;

		VulkanPipeline() = delete;
		VulkanPipeline(VkDevice device);
		~VulkanPipeline();

		void CreateLayout(const VkDescriptorSetLayout& setLayout);
		void LoadShader(VkShaderStageFlagBits flags, VkShaderModule& shader);
		void CreatePipeline();

		VkPipeline GetPipeline() const;
		VkPipelineLayout GetLayout() const;

		static bool LoadShaderModule(VkDevice device, const char* filePath, VkShaderModule* shader);

	private:

		VkPipelineLayout layout;
		VkPipeline pipeline;
		VkPipelineShaderStageCreateInfo shaderInfo;
	};
}