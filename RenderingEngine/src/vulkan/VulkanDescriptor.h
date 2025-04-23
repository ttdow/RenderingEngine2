#pragma once

#include <vulkan/vulkan.h>

namespace Engine
{
	class VulkanDescriptor
	{
	public:

		VulkanDescriptor() = delete;
		VulkanDescriptor(VkDevice device);
		~VulkanDescriptor();

		void AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count = 1);
		void ClearBindings();
		void Build(VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		void InitializePool(uint32_t maxSets);
		void ClearDescriptors() const;
		void Allocate();

		VkDescriptorSet& GetSet();
		VkDescriptorSetLayout GetLayout() const;

	private:

		VkDevice device;
		VkDescriptorPool pool;
		VkDescriptorSetLayout layout;
		VkDescriptorSet set;

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::unordered_map<VkDescriptorType, uint32_t> descriptorTypes;
	};
}