#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "vk_device.h"
#include <glfw/glfw3.h>

namespace flower { namespace graphics{

	class vk_swapchain
	{
	public:
		vk_swapchain(){}
		~vk_swapchain(){}
		operator VkSwapchainKHR()
		{
			return swapchain;
		}

		void initialize(vk_device in_device,VkSurfaceKHR in_surface,GLFWwindow* in_window);
		void destroy();

		const VkExtent2D& get_swapchain_extent() const { return swapchain_extent; }
		auto& get_images() { return swapchain_images; }
		const VkFormat& get_swapchain_image_format() const { return swapchain_imageFormat;}
		std::vector<VkImageView>& get_imageViews() { return swapchain_imageViews; }
		VkSwapchainKHR& get_instance(){ return swapchain;}

	private:
		void create_swapchain();

		// 选择合适的交换链格式
		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats); 

		// 选择合适的显示模式
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		vk_device device;
		VkSurfaceKHR surface;
		GLFWwindow* window;

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_imageViews;
		VkFormat swapchain_imageFormat;
		VkExtent2D swapchain_extent;
		VkSwapchainKHR swapchain;
	};

}}