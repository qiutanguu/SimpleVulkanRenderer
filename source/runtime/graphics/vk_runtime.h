#pragma once

#include <vulkan/vulkan.h>
#include <core/core.h>
#include <vector>
#include <glfw/glfw3.h>
#include "vk/vk_instance.h"
#include "core/camera.h"
#include "core/interface.h"
#include "vk/vk_swapchain.h"
#include "vk/vk_command_buffer.h"
#include "scene_textures.h"
#include "texture_manager.h"
#include "ui/ui_overlay.h"

namespace flower { namespace graphics{

	class vk_runtime : public iruntime_module
	{
	public:
		// 开启的实例插件
		std::vector<const char*> instance_exts = {

		};

		// 开启的实例层
		std::vector<const char*> instance_layers = {

		};

		// 开启的设备特性
		VkPhysicalDeviceFeatures features{ };

		// 开启的设备插件
		std::vector<const char*> device_extensions = 
		{

		};

		virtual void config_before_init() = 0;

		virtual void initialize() override final;
		virtual void initialize_special() = 0;

		virtual void after_initialize() override { /* do nothing */ }

		virtual void tick(float time, float delta_time) override = 0;

		virtual void before_destroy() { vkDeviceWaitIdle(device); }

		virtual void destroy() override final;
		virtual void destroy_special() = 0;

		virtual void recreate_swapchain() = 0;
		virtual void cleanup_swapchain() = 0;

	public:
		vk_runtime(GLFWwindow* window) : window(window),instance({}),device({}) { }
		vk_runtime() {}
		vk_runtime& operator=(const vk_runtime& lhs)
		{
			if(this!=&lhs)
			{
				this->window = lhs.window;
				this->surface = lhs.surface;
			}
			return *this;
		}

		virtual ~vk_runtime() { }

	protected:
		void recreate_swapchain_default();
		void cleanup_swapchain_default();
		

		void submit(std::shared_ptr<vk_command_buffer> buffer_commit,const std::vector<VkSemaphore>& wait_signals);
		void present();
		uint32_t acquire_next_present_image();

	private:
		void create_command_buffers();
		void destroy_command_buffers();

		void create_command_pool();
		void destroy_command_pool();

		void create_sync_objects();
		void destroy_sync_objects();

	protected:
		GLFWwindow* window;
		VkSurfaceKHR surface;

		vk_instance instance;
		vk_device device;
		vk_swapchain swapchain;

		// 图形管线的Command Pool
		VkCommandPool graphics_command_pool;

		// 命令缓冲
		std::vector<std::shared_ptr<vk_command_buffer>> graphics_command_buffers;

		uint32_t image_index;
		uint32_t current_frame = 0;
		const int MAX_FRAMES_IN_FLIGHT = 3; // 同时处理的帧数
		std::vector<VkSemaphore> semaphores_image_available;
		std::vector<VkSemaphore> semaphores_render_finished;
		std::vector<VkFence> inFlight_fences;
		std::vector<VkFence> images_inFlight;

	protected:
		bool framebuffer_resized = false;
		int last_width;
		int last_height;
		friend class application;
	};
}}