#pragma once
#include "glfw/glfw3.h"
#include "vulkan/vulkan.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "../vk/vk_device.h"

namespace flower { namespace graphics{
	
	class ui_context
	{
	public:
		ui_context(
			GLFWwindow* inWindow,
			vk_device* inDevice,
			VkSurfaceKHR inSurface,
			VkInstance inInstance,
			VkCommandPool in_command_pool) : 
			window(inWindow),
			device(inDevice),
			surface(inSurface),
			instance(inInstance),
			command_pool(in_command_pool)
		{
			
		}
		~ui_context() { }

		void initialize(uint32_t back_buffer_counts);

		void draw_frame();
		void destroy();
	


	private:
		VkDescriptorPool descriptor_pool;
		VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	private:
		void create_descriptor_pool();
		void destroy_descriptor_pool();
		
		
	private:
		GLFWwindow* window;
		VkInstance instance;
		VkCommandPool command_pool;
		vk_device* device;

		VkSurfaceKHR surface;
		const uint32_t min_image_count = 2u;

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f,0.55f,0.60f,1.00f);
	};

}}