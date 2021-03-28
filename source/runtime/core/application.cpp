#include "core.h"
#include "application.h"
#include "graphics/context.h"

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

		// 创建图形内容
		graphics_context = graphics::context(window);
		graphics_context.initialize();

		scene_view_cam = camera(glm::vec3(0.0f, 0.0f, 3.0f));
		scene_view_cam.lastX = this->width / 2.0f;
		scene_view_cam.lastY = this->height / 2.0f;
	}

	void application::destroy()
	{
		// 释放图形内容
		graphics_context.destroy();

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void application::loop()
	{
		

		while (!glfwWindowShouldClose(window)) 
		{
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			
			processInput(window);
			graphics_context.draw(scene_view_cam);


			glfwPollEvents();
		}

		// 确保所有图形内容全部完成再退出
		graphics_context.wait_idle();
	}

	void application::framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));

		app->width = width;
		app->height = height;

		app->graphics_context.vk_render_context.framebuffer_resized  = true;
	}

	void application::mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));
		auto& cam = app->scene_view_cam;

		if (cam.firstMouse)
		{
			cam.lastX = xpos;
			cam.lastY = ypos;
			cam.firstMouse = false;
		}

		float xoffset = xpos - cam.lastX;
		float yoffset = cam.lastY - ypos; 

		cam.lastX = xpos;
		cam.lastY = ypos;

		cam.ProcessMouseMovement(xoffset, yoffset);
	}

	void application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto app = reinterpret_cast<application*>(glfwGetWindowUserPointer(window));
		auto& cam = app->scene_view_cam;

		cam.ProcessMouseScroll(yoffset);
	}

	void application::processInput(GLFWwindow *window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::forward, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::backward, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::left, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			scene_view_cam.ProcessKeyboard(camera_utils::move_type::right, deltaTime);
	}
}