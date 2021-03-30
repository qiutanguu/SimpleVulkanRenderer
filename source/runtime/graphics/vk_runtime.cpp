#include "vk_runtime.h"

namespace flower{ namespace graphics{

	void vk_runtime::initialize()
	{
		config_before_init();
		instance.initialize(instance_exts,instance_layers,vk_version::vk_1_0);

		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("窗口表面创建失败！");
		}

		device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		device.initialize(instance,surface,features,device_extensions);
		swapchain.initialize(device,surface,window);

		create_render_pass();
		create_depth_resources();
		create_frame_buffers();
		create_command_pool();
		create_command_buffers();

		initialize_special();
		
		// 记录尺寸大小
		glfwGetWindowSize(window, &last_width, &last_height);
	}

	void vk_runtime::destroy()
	{
		destroy_special();
		
		destroy_depth_resources();
		destroy_frame_buffers();
		destroy_command_buffers();
		destroy_render_pass();
		destroy_command_pool();

		swapchain.destroy();
		device.destroy();

		if(surface!=VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}

		instance.destroy();
	}

	void vk_runtime::recreate_swapchain_default()
	{
		// 最小化时不做任何事情
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 ||height == 0) 
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);
		cleanup_swapchain();

		swapchain.initialize(device,surface,window);
		create_render_pass();
		create_depth_resources();
		create_frame_buffers();
		create_command_buffers();
	}

	void vk_runtime::cleanup_swapchain_default()
	{
		destroy_depth_resources();
		destroy_frame_buffers();
		destroy_command_buffers();
		destroy_render_pass();
		swapchain.destroy();
	}

	void vk_runtime::create_command_buffers()
	{
		graphics_command_buffers.resize(0);

		for (size_t i = 0;i < swapchain_framebuffers.size();i++)
		{
			graphics_command_buffers.push_back(vk_command_buffer::create(
				device,
				graphics_command_pool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				device.graphics_queue
			));
		}
	}

	void vk_runtime::draw_default()
	{
		int current_width;
		int current_height;
		glfwGetWindowSize(window, &current_width, &current_height);

		if(current_width!=last_width||current_height!=last_height)
		{
			last_width = current_width;
			last_height = current_height;
			framebuffer_resized = true;
		}

		// 检查交换链
		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores_image_available[current_frame], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) 
		{
			recreate_swapchain();
			return;
		} 
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
		{
			LOG_VULKAN_FATAL("请求交换链图片失败!");
		}

		update_before_commit(image_index);

		std::vector<VkSemaphore> wait_semaphores = {
			semaphores_image_available[current_frame]
		};

		std::vector<VkPipelineStageFlags> wait_stages = { 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
		};

		graphics_command_buffers[image_index]->wait_flags = wait_stages;
		graphics_command_buffers[image_index]->wait_semaphores = wait_semaphores;
		graphics_command_buffers[image_index]->submit();

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = wait_semaphores.data();

		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &image_index;

		result = vkQueuePresentKHR(device.present_queue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) 
		{
			framebuffer_resized = false;
			recreate_swapchain();
		} 
		else if (result != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("显示交换链图片失败！");
		}
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void vk_runtime::destroy_command_buffers()
	{
		graphics_command_buffers.resize(0);
	}

	void vk_runtime::create_command_pool()
	{
		auto queueFamilyIndices = device.find_queue_families();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family;
		poolInfo.flags = 0;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphics_command_pool) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建图形CommandPool失败！");
		}
	}

	void vk_runtime::destroy_command_pool()
	{
		vkDestroyCommandPool(device, graphics_command_pool, nullptr);
	}

	void vk_runtime::destroy_depth_resources()
	{
		vkDestroyImageView(device, depth_resource.depth_imageView, nullptr);
		vkDestroyImage(device, depth_resource.depth_image, nullptr);
		vkFreeMemory(device, depth_resource.depth_image_memory, nullptr);
	}

	void vk_runtime::create_depth_resources()
	{
		VkFormat depthFormat = find_depth_format(device.physical_device);
		const auto& extent = swapchain.get_swapchain_extent();
		create_image(
			extent.width, 
			extent.height, 
			depthFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			depth_resource.depth_image, 
			depth_resource.depth_image_memory,
			device);

		depth_resource.depth_imageView = create_imageView(
			&depth_resource.depth_image, 
			depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			device);
	}

	void vk_runtime::create_frame_buffers()
	{
		const auto& swap_num = swapchain.get_imageViews().size();
		const auto& extent = swapchain.get_swapchain_extent();

		swapchain_framebuffers.resize(swap_num);

		for (size_t i = 0; i < swap_num; i++)
		{
			std::array<VkImageView, 2> attachments = 
			{
				swapchain.get_imageViews()[i],
				depth_resource.depth_imageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

			// 在这里绑定对应的 renderpass
			framebufferInfo.renderPass = render_pass; 

			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS) 
			{
				LOG_VULKAN_FATAL("创建VulkanFrameBuffer失败！");
			}
		}
	}

	void vk_runtime::destroy_frame_buffers()
	{
		for (auto& SwapChainFramebuffer : swapchain_framebuffers) 
		{
			vkDestroyFramebuffer(device, SwapChainFramebuffer, nullptr);
		}
	}

	void vk_runtime::destroy_render_pass()
	{
		vkDestroyRenderPass(device, render_pass, nullptr);
	}

	void vk_runtime::create_render_pass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapchain.get_swapchain_image_format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = find_depth_format(device.physical_device);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建renderPass失败！");
		}
	}
}}


