#include "pch.h"

#include "VulkanPipeline.h"

namespace Engine
{
	VulkanPipeline::VulkanPipeline(VkDevice device) : device(device)
	{
		layout = nullptr;
		pipeline = nullptr;
		shaderInfo = {};
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, layout, nullptr);
	}

	void VulkanPipeline::CreateLayout(const VkDescriptorSetLayout& setLayout)
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.pSetLayouts = &setLayout;
		info.setLayoutCount = 1;

		// TODO: Push constants.

		info.pPushConstantRanges = nullptr;
		info.pushConstantRangeCount = 0;

		VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &layout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline!");
		}
	}

	void VulkanPipeline::LoadShader(VkShaderStageFlagBits flags, VkShaderModule& shader)
	{
		shaderInfo = {};
		shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo.pNext = nullptr;
		shaderInfo.stage = flags;
		shaderInfo.module = shader;
		shaderInfo.pName = "main";
	}

	void VulkanPipeline::CreatePipeline()
	{
		VkComputePipelineCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.layout = layout;
		createInfo.stage = shaderInfo;

		if (device)
		{
			std::cout << "Seems good.\n";
		}

		VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create compute pipeline!");
		}

		if (pipeline)
		{
			std::cout << "Seems okay.\n";
		}
	}

	VkPipeline VulkanPipeline::GetPipeline() const
	{
		return pipeline;
	}

	VkPipelineLayout VulkanPipeline::GetLayout() const
	{
		return layout;
	}

	bool VulkanPipeline::LoadShaderModule(VkDevice device, const char* filePath, VkShaderModule* shader)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		
		if (!file.is_open())
		{
			return false;
		}

		size_t fileSize = static_cast<size_t>(file.tellg());

		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		VkShaderModuleCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pNext = nullptr;
		info.codeSize = buffer.size() * sizeof(uint32_t);
		info.pCode = buffer.data();

		VkResult result = vkCreateShaderModule(device, &info, nullptr, shader);
		if (result != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}
}