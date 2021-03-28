#pragma once
#include <vulkan/vulkan.h>
#include <core/core.h>
#include <vector>
#include <glfw/glfw3.h>
#include "device.h"
#include "instance.h"
#include "render_context.h"
#include "core/camera.h"

namespace flower { namespace graphics{

	class context
	{
	public:
		context(GLFWwindow* window);
		context() {};


		context& operator=(const context& lhs)
		{
			if(this!=&lhs)
			{
				this->window = lhs.window;
				this->surface = lhs.surface;
			}
			return *this;
		}

		~context() { }

		void initialize();
		void destroy();
		void draw(camera& cam);

		// 等待所有的图形任务完成
		void wait_idle();

		

		render_context vk_render_context;
	private:
		GLFWwindow* window;
		VkSurfaceKHR surface;

		instance vk_instance;
		device vk_device;
		
	};
	

}}