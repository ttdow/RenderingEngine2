#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Engine
{
	class VulkanImage
	{
	public:

		VulkanImage() = delete;
		VulkanImage(VkDevice device, VmaAllocator allocator, VkFormat format, VkExtent2D extent, VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags);
		~VulkanImage();

		VkImage GetImage() const;
		VkImageView GetView() const;

		static void CopyImageToImage(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);

	private:

		VkImage image;
		VkImageView view;

		VkDevice device;
		VmaAllocator allocator;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkFormat format;
		VkExtent2D extent;
	};
}