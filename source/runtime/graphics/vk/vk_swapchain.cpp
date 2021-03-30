#include "vk_swapchain.h"
#include "vk_device.h"
#include <glfw/glfw3.h>
#include "core/core.h"

namespace flower { namespace graphics{
	
	void vk_swapchain::initialize(vk_device in_device,VkSurfaceKHR in_surface,GLFWwindow* in_window)
	{
		device = in_device;
		surface = in_surface;
		window = in_window;

		create_swapchain();
	}
	
	void vk_swapchain::destroy()
	{
		for (auto imageView : swapchain_imageViews) 
		{
			vkDestroyImageView(device,imageView,nullptr);
		}
		swapchain_imageViews.resize(0);

		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}

	
	
	void vk_swapchain::create_swapchain()
	{
		auto swapchain_support = device.query_swapchain_support();

		VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapchain_support.formats);
		VkPresentModeKHR presentMode = choose_swap_present_mode(swapchain_support.presentModes);
		VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

		// 至少加载一张图片.
		uint32_t imageCount = swapchain_support.capabilities.minImageCount + 1; 

		// 应确保不超过最大图像数，其中0为特殊值表示没有最大图像数：																	   
		if (swapchain_support.capabilities.maxImageCount > 0 && imageCount > swapchain_support.capabilities.maxImageCount) 
		{
			imageCount = swapchain_support.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // 每个交换层理应只需要一张图片
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto indices = device.find_queue_families();
		uint32_t queueFamilyIndices[] = { indices.graphics_family, indices.present_family };

		if (indices.graphics_family != indices.present_family) 
		{
			// VK_SHARING_MODE_CONCURRENT：
			// 图像可以在多个队列系列中使用，而无需明确的所有权转移。
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} 
		else 
		{
			// VK_SHARING_MODE_EXCLUSIVE： 
			// 图像一次由一个队列家族拥有，并且必须在其他队列家族中使用图像之前显式转移所有权。
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// 不希望对图像进行旋转
		createInfo.preTransform = swapchain_support.capabilities.currentTransform;

		// 是否应将Alpha通道用于与窗口系统中的其他窗口混合
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;

		// VK_TRUE则意味着我们不在乎被遮盖的像素的颜色
		createInfo.clipped = VK_TRUE; 

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("交换链创建失败！");
		}

		// 申请图片
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		swapchain_images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchain_images.data());

		swapchain_imageFormat = surfaceFormat.format;
		swapchain_extent = extent;

		// 在这里创建SwapChain需要的ImageViews
		swapchain_imageViews.resize(swapchain_images.size());
		for (size_t i = 0; i < swapchain_images.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain_images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 交换链一般为2D图片
			createInfo.format = swapchain_imageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// 记得销毁
			if (vkCreateImageView(device, &createInfo, nullptr, &swapchain_imageViews[i]) != VK_SUCCESS) 
			{
				LOG_VULKAN_FATAL("创建交换链图片视图失败！");
			}
		}

		LOG_VULKAN_TRACE("创建Vulkan SwapChain成功，BackBuffer数{0}。",swapchain_imageViews.size());
	}

	VkSurfaceFormatKHR vk_swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR vk_swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) 
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D vk_swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) 
		{
			return capabilities.currentExtent;
		} 
		else 
		{
			int width, height;
			glfwGetFramebufferSize(window,&width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
	
}}