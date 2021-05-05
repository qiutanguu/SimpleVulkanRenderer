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
		constexpr auto GS = "main";
		constexpr auto TSE = "main";
		constexpr auto TSC = "main";
		constexpr auto CS = "main";
	}

	// 交换链支持细节
	struct vk_swapchain_support_details 
	{
		// 基本表面功能（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
		VkSurfaceCapabilitiesKHR capabilities; 

		// 表面格式（像素格式，色彩空间）
		std::vector<VkSurfaceFormatKHR> formats; 

		// 可用的演示模式
		std::vector<VkPresentModeKHR> presentModes;
	};

	// 队列族对应的次序
	class vk_queue_family_indices 
	{
		friend class vk_device;
	public:
		uint32_t graphics_family; // 图形队列族
		uint32_t present_family;  // 显示队列族
		uint32_t compute_faimly;  // 计算队列族

		bool is_completed() 
		{
			return graphics_family_set && present_family_set && compute_faimly_set;
		}
	private:
		bool graphics_family_set = false;
		bool present_family_set = false;
		bool compute_faimly_set = false;
	};

	inline VkImageView create_imageView(VkImage* image,VkFormat format, VkImageAspectFlags aspectFlags,VkDevice device,uint32_t mipMaplevels = 1)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = *image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipMaplevels;
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
	inline VkFormat find_supported_format(VkPhysicalDevice gpu,const std::vector<VkFormat>& candidates,VkImageTiling tiling,VkFormatFeatureFlags features)
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
	inline VkFormat find_depth_format(VkPhysicalDevice physical_device) 
	{
		return find_supported_format
		(
			physical_device,
			{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// 寻找仅深度格式（用于shadowmap）
	inline VkFormat find_depthonly_format(VkPhysicalDevice physical_device) 
	{
		return find_supported_format
		(
			physical_device,
			{ VK_FORMAT_D32_SFLOAT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// 单个命令记录开始 见：end_single_time_commands copyBufferToImage
	inline VkCommandBuffer begin_single_time_commands(VkCommandPool pool,VkDevice device) 
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


	// 单个命令结束 见：begin_single_time_commands copyBufferToImage
	inline void end_single_time_commands(
		VkCommandBuffer commandBuffer,
		VkQueue& queue,
		VkCommandPool pool,
		VkDevice device) 
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
	}

	// 将Buffer数据复制到图片中
	inline void copy_buffer_to_image(VkBuffer buffer,VkImage image,uint32_t width,uint32_t height,VkCommandPool commandpool,VkDevice device,VkQueue in_graphics_queue)
	{
		VkCommandBuffer commandBuffer = begin_single_time_commands(commandpool,device);

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

		end_single_time_commands(commandBuffer,in_graphics_queue,commandpool,device);
	}

	inline void generate_mipmaps(VkImage image, VkFormat imageFormat,int32_t texWidth,int32_t texHeight,uint32_t mipLevels,VkCommandPool commandpool,VkDevice device,VkQueue in_graphics_queue,VkPhysicalDevice gpu)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(gpu,imageFormat,&formatProperties);

		if(!(formatProperties.optimalTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			LOG_VULKAN_FATAL("纹理格式不支持线性插值！");
		}

		VkCommandBuffer commandBuffer = begin_single_time_commands(commandpool,device);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for(uint32_t i = 1; i<mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i-1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,
				0,nullptr,
				0,nullptr,
				1,&barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i-1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {mipWidth>1 ? mipWidth/2 : 1, mipHeight>1 ? mipHeight/2 : 1, 1};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,&blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,
				0,nullptr,
				0,nullptr,
				1,&barrier);

			if(mipWidth>1) mipWidth /= 2;
			if(mipHeight>1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels-1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,
			0,nullptr,
			0,nullptr,
			1,&barrier);

		end_single_time_commands(commandBuffer,in_graphics_queue,commandpool,device);
	}

	inline void transition_image_layout(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkCommandPool commandpool,
		VkDevice device,
		VkQueue in_graphics_queue,
		uint32_t mipmapLevels = 1)
	{
		VkCommandBuffer commandBuffer = begin_single_time_commands(commandpool,device);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipmapLevels;
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
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
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

		end_single_time_commands(commandBuffer,in_graphics_queue,commandpool,device);
	}

	void create_texture2D(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory,
		class vk_device& in_device,
		uint32_t miplevels = 1);

	inline void create_texture2D_imageView(VkImage* image,VkImageView* view,VkDevice device)
	{
		*view = create_imageView(image, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT,device);
	}
}}
