#pragma once
#include <vulkan/vulkan.h>
#include "core/core.h"

namespace flower { namespace graphics {

	template <class T>
	inline std::string vulkan_to_string(const T &value)
	{
		std::stringstream ss;
		ss << std::fixed << value;
		return ss.str();
	}

	class vk_exception : public std::runtime_error
	{
	public:
		vk_exception(VkResult result,const std::string &msg = "vulkan exception"):
			result {result}, std::runtime_error {msg}
		{
			error_message = std::string(std::runtime_error::what())+std::string{" : "} + vulkan_to_string(result);
		}

		const char *what() const noexcept override
		{
			return error_message.c_str();
		}

		VkResult result;
	private:
		std::string error_message;
	};

#ifdef FLOWER_DEBUG
	inline void vk_check(VkResult err)
	{
		if (err)
		{ 
			LOG_VULKAN_FATAL("检查错误: {}", vulkan_to_string(err));
			__debugbreak(); 
			abort(); 
		} 
	}

	inline void assert_vk_handle(decltype(VK_NULL_HANDLE) handle)
	{
		if (handle == VK_NULL_HANDLE) 
		{ 
			LOG_VULKAN_FATAL("句柄为空！");
			__debugbreak(); 
			abort(); 
		}
	}
#else
	inline void vk_check(VkResult err)
	{
	}

	inline void assert_vk_handle(decltype(VK_NULL_HANDLE) handle)
	{
	}
#endif // FLOWER_DEBUG

	namespace shader_main_function_name
	{
		constexpr auto VS = "main";
		constexpr auto FS = "main";
	}

	class shader_module : non_copyable
	{
	public:
		shader_module(VkDevice& in_device,const std::vector<char>& code) : device(in_device)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			if (vkCreateShaderModule(device, &createInfo, nullptr, &handle) != VK_SUCCESS) 
			{
				LOG_VULKAN_ERROR("创建shader模块失败！");
			}
		}

		~shader_module()
		{
			vkDestroyShaderModule(device, handle, nullptr);
		}

		VkShaderModule& get_handle() { return handle; }

	private:
		VkShaderModule handle {VK_NULL_HANDLE};
		VkDevice& device;
	};

	inline VkImageView create_imageView(VkImage* image,VkFormat format, VkImageAspectFlags aspectFlags,VkDevice device)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = *image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.aspectMask = aspectFlags;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建图片视图失败！");
		}

		return imageView;
	}

	// 寻找支持的格式
	inline VkFormat findSupportedFormat(VkPhysicalDevice gpu,const std::vector<VkFormat>& candidates,VkImageTiling tiling,VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			} 
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
				return format;
			}
		}
		LOG_VULKAN_FATAL("找不到支持的格式！");
	}

	// 寻找深度格式
	inline VkFormat findDepthFormat(VkPhysicalDevice gpu) 
	{
		return findSupportedFormat
		(
			gpu,
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// 单个命令记录开始 见：endSingleTimeCommands copyBufferToImage
	inline VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool,VkDevice device) 
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}


	// 单个命令结束 见：beginSingleTimeCommands copyBufferToImage
	inline void endSingleTimeCommands(VkCommandBuffer commandBuffer,VkQueue& graphicsQueue,VkCommandPool pool,VkDevice device) 
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
	}

	// 将Buffer数据复制到图片中
	inline void copyBufferToImage(VkBuffer buffer,VkImage image,uint32_t width,uint32_t height,VkCommandPool commandpool,VkDevice device,VkQueue in_graphics_queue)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandpool,device);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer,in_graphics_queue,commandpool,device);
	}

	inline void transitionImageLayout(VkImage image,VkFormat format,VkImageLayout oldLayout,VkImageLayout newLayout,VkCommandPool commandpool,VkDevice device,VkQueue in_graphics_queue)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandpool,device);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if 
		(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} 
		else 
		{
			LOG_VULKAN_FATAL("不支持的图片布局转化！");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer,in_graphics_queue,commandpool,device);
	}

	void createImage(uint32_t width,uint32_t height,VkFormat format,VkImageTiling tiling,VkImageUsageFlags usage,VkMemoryPropertyFlags properties,VkImage& image,VkDeviceMemory& imageMemory,class device& in_device);

	inline void createTexture2DImageView(VkImage* image,VkImageView* view,VkDevice device)
	{
		*view = create_imageView(image, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT,device);
	}
}}
