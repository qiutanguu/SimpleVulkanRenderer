#include "vk_runtime.h"
#include "shader_manager.h"
#include "global_uniform_buffers.h"
#include "scene/mesh.h"
#include "../core/timer.h"

namespace flower{ namespace graphics{

	void vk_runtime::initialize()
	{
		global_timer::reset();
		config_before_init();
		instance.initialize(instance_exts,instance_layers,vk_version::vk_1_0);

		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("窗口表面创建失败！");
		}

		device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		device.initialize(instance,surface,features,device_extensions);
		swapchain.initialize(device,surface,window);

		create_command_pool();
		create_command_buffers();
		create_sync_objects();

		g_texture_manager.initialize(&device,graphics_command_pool);
		g_scene_textures.initialize(&device,&swapchain);
		g_shader_manager.initialize(&device);
		g_uniform_buffers.initialize(&device,&swapchain,graphics_command_pool);
		g_meshes_manager.initialize(&device,graphics_command_pool);
		
		initialize_special();
		glfwGetWindowSize(window, &last_width, &last_height);
	}

	void vk_runtime::destroy()
	{
		destroy_special();

		g_texture_manager.release();
		g_scene_textures.release();
		g_shader_manager.release();
		g_uniform_buffers.release();
		g_meshes_manager.release();

		destroy_command_buffers();
		destroy_sync_objects();
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

		// 重新申请所有的rt
		g_scene_textures.release();
		g_scene_textures.initialize(&device,&swapchain);

		g_uniform_buffers.release();
		g_uniform_buffers.initialize(&device,&swapchain,graphics_command_pool);

		create_command_buffers();

		images_inFlight.resize(swapchain.get_imageViews().size(), VK_NULL_HANDLE);
	}

	void vk_runtime::create_sync_objects()
	{
		const auto image_nums = swapchain.get_images().size();

		// 为每个处理中的帧添加同步讯号
		semaphores_image_available.resize(MAX_FRAMES_IN_FLIGHT);
		semaphores_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
		inFlight_fences.resize(MAX_FRAMES_IN_FLIGHT);
		images_inFlight.resize(image_nums,VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for(size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++)
		{
			if(
				vkCreateSemaphore(device,&semaphoreInfo,nullptr,&semaphores_image_available[i])!=VK_SUCCESS||
				vkCreateSemaphore(device,&semaphoreInfo,nullptr,&semaphores_render_finished[i])!=VK_SUCCESS||
				vkCreateFence(device,&fenceInfo,nullptr,&inFlight_fences[i])!=VK_SUCCESS)
			{
				LOG_VULKAN_FATAL("为帧对象创建同步对象时出错！");
			}
		}
	}

	void vk_runtime::destroy_sync_objects()
	{
		for(size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device,semaphores_image_available[i],nullptr);
			vkDestroySemaphore(device,semaphores_render_finished[i],nullptr);
			vkDestroyFence(device,inFlight_fences[i],nullptr);
		}
	}

	void vk_runtime::cleanup_swapchain_default()
	{
		destroy_command_buffers();
		swapchain.destroy();
	}

	void vk_runtime::create_command_buffers()
	{
		graphics_command_buffers.resize(0);

		for (size_t i = 0;i < swapchain.get_imageViews().size();i++)
		{
			graphics_command_buffers.push_back(vk_command_buffer::create(
				device,
				graphics_command_pool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				device.graphics_queue
			));
		}
	}

	void vk_runtime::submit(std::shared_ptr<vk_command_buffer> buffer_commit)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// 等待交换链图片可用。
		std::vector<VkSemaphore> wait_semaphores = { semaphores_image_available[current_frame] };
		std::vector<VkPipelineStageFlags> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = (uint32_t)wait_semaphores.size();
		submitInfo.pWaitSemaphores = wait_semaphores.data();
		submitInfo.pWaitDstStageMask = wait_stages.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer_commit->get_instance();

		VkSemaphore signalSemaphores[] = { semaphores_render_finished[current_frame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device,1,&inFlight_fences[current_frame]);

		if(vkQueueSubmit(buffer_commit->get_queue(),1,&submitInfo,inFlight_fences[current_frame]) != VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("提交CommandBuffer失败！");
		}
	}

	void vk_runtime::present()
	{
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore signalSemaphores[] = { semaphores_render_finished[current_frame] };
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { swapchain };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;

		present_info.pImageIndices = &image_index;

		auto result = vkQueuePresentKHR(device.present_queue,&present_info);

		if(result==VK_ERROR_OUT_OF_DATE_KHR || result==VK_SUBOPTIMAL_KHR || framebuffer_resized)
		{
			framebuffer_resized = false;
			recreate_swapchain();
		}
		else if(result!=VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("显示交换链图片失败！");
		}

		current_frame = (current_frame +1) % MAX_FRAMES_IN_FLIGHT;
	}

	uint32_t vk_runtime::acquire_next_present_image()
	{
		// 检查窗口大小是否改变
		int current_width;
		int current_height;
		glfwGetWindowSize(window,&current_width,&current_height);

		if(current_width!=last_width||current_height!=last_height)
		{
			last_width = current_width;
			last_height = current_height;
			framebuffer_resized = true;
		}

		vkWaitForFences(device,1,&inFlight_fences[current_frame],VK_TRUE,UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,semaphores_image_available[current_frame],VK_NULL_HANDLE,&image_index);

		if(result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
		}
		else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			LOG_VULKAN_FATAL("请求交换链图片失败!");
		}

		if(images_inFlight[image_index]!=VK_NULL_HANDLE)
		{
			vkWaitForFences(device,1,&images_inFlight[image_index],VK_TRUE,UINT64_MAX);
		}

		images_inFlight[image_index] = inFlight_fences[current_frame];

		return image_index;
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

}}


