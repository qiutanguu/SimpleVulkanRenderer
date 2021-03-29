#pragma once
#include <vulkan/vulkan.h>
#include <core/core.h>
#include <vector>
#include <glfw/glfw3.h>
#include "vk/vk_device.h"
#include "vk/vk_instance.h"
#include "core/camera.h"
#include "core/interface.h"
#include "vk/vk_swapchain.h"

namespace flower { namespace graphics{

	class vk_runtime : public iruntime_module
	{
	public:
		// ������ʵ�����
		std::vector<const char*> instance_exts = {

		};

		// ������ʵ����
		std::vector<const char*> instance_layers = {

		};

		// �������豸����
		VkPhysicalDeviceFeatures features{ };

		// �������豸���
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

		// TODO: �ؽ�������û���ҵ�̫�õĳ��󷽷���
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
		size_t current_frame = 0;

		// Ĭ�ϻ��ƺ�����ͬʱ�ṩ�麯��update_before_commit�����м��Ա����ubo
		void draw_default();
		// �� draw_default
		virtual void update_before_commit(uint32_t backBuffer_index) {}

	private:
		void create_command_buffers();
		void destroy_command_buffers();

		void create_frame_buffers();
		void destroy_frame_buffers();
		
		void create_command_pool();
		void destroy_command_pool();

		void create_depth_resources();
		void destroy_depth_resources();

		void create_render_pass();
		void destroy_render_pass();

		void create_sync_objects();
		void destroy_sync_objects();

	protected:
		GLFWwindow* window;
		VkSurfaceKHR surface;

		vk_instance instance;
		vk_device device;
		vk_swapchain swapchain;
		vk_depth_resource depth_resource;

		// ͼ�ι��ߵ�Command Pool
		VkCommandPool graphics_command_pool;

		// Ĭ����ȾPass
		VkRenderPass render_pass;

		// FrameBuffers
		std::vector<VkFramebuffer> swapchain_framebuffers;

		// �����
		std::vector<VkCommandBuffer> graphics_command_buffers;

		// ͬ��
		const int MAX_FRAMES_IN_FLIGHT = 2; // ͬʱ�����֡��
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