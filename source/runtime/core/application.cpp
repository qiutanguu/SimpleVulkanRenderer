#include "core.h"
#include "application.h"

namespace flower
{
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
			window = glfwCreateWindow(width,height,title,nullptr,nullptr);
		}

		glfwSetWindowUserPointer(window,this);
		glfwSetFramebufferSizeCallback(window,framebuffer_resize_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		// 禁用鼠标图标
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		this->width = width;
		this->height = height;

		LOG_INFO("创建应用大小为宽：{0}，高：{1}。",this->width,this->height);

		scene_view_cam = camera(glm::vec3(0.0f, 0.0f, 3.0f));
		scene_view_cam.lastX = this->width / 2.0f;
		scene_view_cam.lastY = this->height / 2.0f;
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
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));
		auto& cam = app->scene_view_cam;

		if (cam.firstMouse)
		{
			cam.lastX = (float)xpos;
			cam.lastY = (float)ypos;
			cam.firstMouse = false;
		}

		float xoffset = (float)xpos - cam.lastX;
		float yoffset = cam.lastY - (float)ypos; 

		cam.lastX = (float)xpos;
		cam.lastY = (float)ypos;

		cam.ProcessMouseMovement(xoffset, yoffset);
	}

	void application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));
		auto& cam = app->scene_view_cam;

		cam.ProcessMouseScroll((float)yoffset);
	}

	void application::processInput(GLFWwindow *window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::forward, delta_time);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::backward, delta_time);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::left, delta_time);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::right, delta_time);
	}
}