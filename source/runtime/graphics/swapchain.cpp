#include "swapchain.h"
#include "device.h"
#include <glfw/glfw3.h>
#include "core/core.h"

namespace flower { namespace graphics{
	
	void swapchain::initialize(device in_device,VkSurfaceKHR in_surface,GLFWwindow* in_window)
	{
		vk_device = in_device;
		surface = in_surface;
		window = in_window;

		createSwapChain();
	}
	
	void swapchain::destroy()
	{
		for (auto imageView : swapChainImageViews) 
		{
			vkDestroyImageView(vk_device.logic_device,imageView,nullptr);
		}

		vkDestroySwapchainKHR(vk_device.logic_device, swapChain, nullptr);
	}
	
	void swapchain::createSwapChain()
	{
		auto swapChainSupport = vk_device.query_swapchain_support();

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// ���ټ���һ��ͼƬ.
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; 

		// Ӧȷ�����������ͼ����������0Ϊ����ֵ��ʾû�����ͼ������																	   
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		// ÿ����������Ӧֻ��Ҫһ��ͼƬ
		createInfo.imageArrayLayers = 1; 
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto indices = vk_device.find_queue_families();
		uint32_t queueFamilyIndices[] = { indices.graphics_family, indices.present_family };

		
		
		if (indices.graphics_family != indices.present_family) 
		{
			// VK_SHARING_MODE_CONCURRENT��
			// ͼ������ڶ������ϵ����ʹ�ã���������ȷ������Ȩת�ơ�
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} 
		else 
		{
			// VK_SHARING_MODE_EXCLUSIVE�� 
			// ͼ��һ����һ�����м���ӵ�У����ұ������������м�����ʹ��ͼ��֮ǰ��ʽת������Ȩ��
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// ��ϣ����ͼ�������ת
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// �Ƿ�Ӧ��Alphaͨ�������봰��ϵͳ�е��������ڻ��
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;

		// VK_TRUE����ζ�����ǲ��ں����ڸǵ����ص���ɫ
		createInfo.clipped = VK_TRUE; 

		if (vkCreateSwapchainKHR(vk_device.logic_device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("����������ʧ�ܣ�");
		}

		vkGetSwapchainImagesKHR(vk_device.logic_device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(vk_device.logic_device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		// �����ﴴ��SwapChain��Ҫ��ImageViews
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			// ������һ��Ϊ2DͼƬ
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			// �ǵ�����
			if (vkCreateImageView(vk_device.logic_device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) 
			{
				LOG_VULKAN_FATAL("����������ͼƬ��ͼʧ�ܣ�");
			}
		}
	}

	VkSurfaceFormatKHR swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

	VkPresentModeKHR swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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


	VkExtent2D swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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