#pragma once
#include "core.h"
#include "interface.h"
#include <glfw/glfw3.h>
#include "camera.h"
#include <memory>

namespace flower
{
	class application : non_copyable
	{
	public:
		explicit application() { }
		~application() { }

		void initialize(uint32_t width = 1280u,uint32_t height = 960u,const char* title = "flower",bool full_screen = false);
		void initialize_modules();
		void loop();
		void destroy();

		auto get_window() { return window; }
		std::vector< std::shared_ptr<iruntime_module>> modules = {};

	protected:
		static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		void processInput(GLFWwindow *window);

	protected:
		GLFWwindow* window;
		
		uint32_t width = 1280u;
		uint32_t height = 960u;

		// timing
		float delta_time = 0.0f;
		float last_time = 0.0f;
	};
}