#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "device.h"
#include <glfw/glfw3.h>

namespace flower { namespace graphics{

	class swapchain
	{
	public:
		swapchain(){}
		~swapchain(){}

		void initialize(device in_device,VkSurfaceKHR in_surface,GLFWwindow* in_window);
		void destroy();

		const VkExtent2D& GetSwapChainExtent() const { return swapChainExtent; }
		auto& GetImages() { return swapChainImages; }
		const VkFormat& GetSwapChainImageFormat() const { return swapChainImageFormat;}
		std::vector<VkImageView>& GetImageViews() { return swapChainImageViews; }
		VkSwapchainKHR& GetInstance(){ return swapChain;}

	private:
		void createSwapChain();

		// 选择合适的交换链格式
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); 

		// 选择合适的显示模式
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		device vk_device;
		VkSurfaceKHR surface;
		GLFWwindow* window;

		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		VkSwapchainKHR swapChain;
	};

}}