#pragma once
#include "core.h"
#include "graphics/context.h"
#include "camera.h"

namespace flower
{
	namespace graphics { class context; }

	class application : non_copyable
	{
	public:
		explicit application() { }
		~application() { }

		void initialize(uint32_t width = 1280u,uint32_t height = 960u,const char* title = "flower",bool full_screen = false);
		void loop();
		void destroy();


	protected:
		static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		void processInput(GLFWwindow *window);

	protected:
		GLFWwindow* window;
		graphics::context graphics_context = {};
		camera scene_view_cam = {};

		uint32_t width = 1280u;
		uint32_t height = 960u;

		// timing
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;
	};
}