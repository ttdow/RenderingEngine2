#include "pch.h"

#include "VulkanDescriptor.h"

namespace Engine
{
	VulkanDescriptor::VulkanDescriptor(VkDevice device) : device(device) 
	{
		pool = nullptr;
		layout = nullptr;
		set = nullptr;
	}

	VulkanDescriptor::~VulkanDescriptor()
	{
		if (pool)
		{
			vkDestroyDescriptorPool(device, pool, nullptr);
		}

		if (layout)
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
		}
	}

	void VulkanDescriptor::AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count)
	{
		VkDescriptorSetLayoutBinding newBinding = {};
		newBinding.binding = binding;
		newBinding.descriptorCount = count; // Defaults to 1.
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = 0; // This will be updated later when we build the layout.

		bindings.push_back(newBinding);

		// Track the count of each descriptor type we bind so we can
		// create an appropriately sized descriptor pool later.
		descriptorTypes[type] += count;
	}

	void VulkanDescriptor::ClearBindings()
	{
		bindings.clear();

		descriptorTypes.clear();
	}

	void VulkanDescriptor::Build(VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
	{
		for (VkDescriptorSetLayoutBinding& binding : bindings)
		{
			binding.stageFlags = shaderStages;
		}

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = pNext;
		info.bindingCount = static_cast<uint32_t>(bindings.size());
		info.pBindings = bindings.data();
		info.flags = flags;

		VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to build descriptor set layout!");
		}
	}

	void VulkanDescriptor::InitializePool(uint32_t maxSets)
	{
		if (descriptorTypes.size() < 1)
		{
			return;
		}

		// Create a pool large enough for each descriptor type that has been bound.
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const std::pair<VkDescriptorType, uint32_t>& typeCount : descriptorTypes)
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = typeCount.first;
			poolSize.descriptorCount = typeCount.second;

			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.maxSets = maxSets;
		info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		info.pPoolSizes = poolSizes.data();

		VkResult result = vkCreateDescriptorPool(device, &info, nullptr, &pool);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Faile to create descriptor pool!");
		}
	}

	void VulkanDescriptor::ClearDescriptors() const
	{
		// When you reset a pool, it destroys all the descriptor sets allocated from it.
		vkResetDescriptorPool(device, pool, 0);
	}

	void VulkanDescriptor::Allocate()
	{
		VkDescriptorSetAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.descriptorPool = pool;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &layout;

		VkResult result = vkAllocateDescriptorSets(device, &info, &set);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor set!");
		}
	}

	VkDescriptorSet& VulkanDescriptor::GetSet()
	{
		return set;
	}

	VkDescriptorSetLayout VulkanDescriptor::GetLayout() const
	{
		return layout;
	}
}