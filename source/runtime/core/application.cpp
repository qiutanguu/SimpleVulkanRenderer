#include "core.h"
#include "application.h"
#include "graphics/global_uniform_buffers.h"
#include "input.h"

namespace flower
{
	camera g_cam = camera(glm::vec3(0.0f, 0.0f, 3.0f));

	void application::initialize(uint32_t width,uint32_t height,const char* title,bool full_screen)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);

		if(full_screen)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

			width = mode->width;
			height = mode->height;
			window = glfwCreateWindow(width,height,title,glfwGetPrimaryMonitor(),nullptr);
			glfwSetWindowPos(window,(mode->width-width)/2,(mode->height-height)/2);
		}
		else
		{
			// NOTE：暂时最大化显示。
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			width = mode->width;
			height = mode->height;
			glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
			window = glfwCreateWindow(width,height,title,nullptr,nullptr);
			// window = glfwCreateWindow(width,height,title,nullptr,nullptr);
		}

		glfwSetWindowUserPointer(window,this);
		glfwSetFramebufferSizeCallback(window,framebuffer_resize_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		this->width = width;
		this->height = height;

		LOG_INFO("创建应用大小为宽：{0}，高：{1}。",this->width,this->height);

		g_cam.lastX = this->width / 2.0f;
		g_cam.lastY = this->height / 2.0f;
	}

	void application::initialize_modules()
	{
		for(auto& runtime_module : modules)
		{
			runtime_module->initialize();
		}
	}

	void application::destroy()
	{
		for(auto& runtime_module : modules)
		{
			runtime_module->destroy();
		}

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void application::loop()
	{
		for(auto& runtime_module : modules)
		{
			runtime_module->after_initialize();
		}

		while (!glfwWindowShouldClose(window)) 
		{
			float current_time = (float)glfwGetTime();
			delta_time = current_time - last_time;
			last_time = current_time;
			
			processInput(window);

			for(auto& runtime_module : modules)
			{
				runtime_module->tick(current_time,delta_time);
			}

			glfwPollEvents();

		}

		for(auto& runtime_module : modules)
		{
			runtime_module->before_destroy();
		}
	}

	void application::framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));

		app->width = width;
		app->height = height;
	}

	void application::mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (g_cam.firstMouse)
		{
			g_cam.lastX = (float)xpos;
			g_cam.lastY = (float)ypos;
			g_cam.firstMouse = false;
		}

		float xoffset = (float)xpos - g_cam.lastX;
		float yoffset = g_cam.lastY - (float)ypos; 

		g_cam.lastX = (float)xpos;
		g_cam.lastY = (float)ypos;

		g_cam.ProcessMouseMovement(xoffset, yoffset);

		input::current_mouse_pos = glm::vec2(float(xpos),float(ypos));
	}

	void application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));
		g_cam.ProcessMouseScroll((float)yoffset);
	}

	void application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if(button==GLFW_MOUSE_BUTTON_RIGHT && action==GLFW_PRESS)
		{
			input::right_mouse_button_down = true;
		}
		else if(button==GLFW_MOUSE_BUTTON_RIGHT && action==GLFW_RELEASE)
		{
			input::right_mouse_button_down = false;
		}
		else if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_RELEASE)
		{
			input::left_mouse_button_down = false;
		}
		else if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)
		{
			input::left_mouse_button_down = true;
		}
	}

	void application::processInput(GLFWwindow *window)
	{
		if(glfwGetKey(window,GLFW_KEY_GRAVE_ACCENT)==GLFW_PRESS)
		{
			if(!input::key_grave_accent_down)
			{
				if(input::disable_cursor)// 第一次按下
				{
					// 启用鼠标图标
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					input::disable_cursor = false;
				}
				else// 第二次按下
				{
					// 禁用鼠标图标
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					input::disable_cursor = true;
				}
			}

			input::key_grave_accent_down = true;
		}
		else if(glfwGetKey(window,GLFW_KEY_GRAVE_ACCENT)==GLFW_RELEASE)
		{
			input::key_grave_accent_down = false;
		}

		if(glfwGetKey(window,GLFW_KEY_TAB)==GLFW_PRESS)
		{
			input::key_tab = true;
		}
		else if(glfwGetKey(window,GLFW_KEY_TAB)==GLFW_RELEASE)
		{
			input::key_tab = false;
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			g_cam.ProcessKeyboard(camera_utils::move_type::forward, delta_time);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			g_cam.ProcessKeyboard(camera_utils::move_type::backward, delta_time);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			g_cam.ProcessKeyboard(camera_utils::move_type::left, delta_time);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			g_cam.ProcessKeyboard(camera_utils::move_type::right, delta_time);

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			graphics::g_uniform_buffers.io_process(delta_time);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			graphics::g_uniform_buffers.io_process(-delta_time);


	}
}