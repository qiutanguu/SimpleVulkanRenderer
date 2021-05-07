#include "vk_common.h"
#include "vk_device.h"

namespace flower{ namespace graphics{

	void create_texture2D(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory,
		vk_device& in_device,
		uint32_t miplevels)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = miplevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		// ����ʹ��EXCLUSIVE��
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(in_device, &imageInfo, nullptr, &image) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("����ͼƬ���ʧ�ܣ�");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(in_device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = in_device.find_memory_type(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(in_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("����ͼƬ�ڴ�ʧ�ܣ�");
		}

		vkBindImageMemory(in_device, image, imageMemory, 0);
	}

} }