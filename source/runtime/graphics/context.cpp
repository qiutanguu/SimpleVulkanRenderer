#include "context.h"

namespace flower{ namespace graphics{

	context::context(GLFWwindow* window) : window(window),vk_instance({}),vk_device({}),vk_render_context({})
	{
	}

	void context::initialize()
	{
		// ������ʵ�����
		std::vector<const char*> instance_exts = {

		};

		// ������ʵ����
		std::vector<const char*> instance_layers = {

		};

		vk_instance.initialize(instance_exts,instance_layers,vk_version::vk_1_0);

		if (glfwCreateWindowSurface(vk_instance.vk_instance, window, nullptr, &surface) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("���ڱ��洴��ʧ�ܣ�");
		}

		// �������豸����
		VkPhysicalDeviceFeatures features{ };
		features.samplerAnisotropy = true;

		// �������豸���
		std::vector<const char*> device_extensions = 
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		vk_device.initialize(vk_instance.vk_instance,surface,features,device_extensions);
		

		// ��ʼ����Ⱦͼ������
		vk_render_context.initialize(vk_device,surface,window);
	}

	void context::destroy()
	{
		vk_render_context.destroy();

		vk_device.destroy();

		if(surface!=VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(vk_instance.vk_instance, surface, nullptr);
		}

		vk_instance.destroy();
	}

	void context::draw(camera& cam)
	{
		this->vk_render_context.draw(cam);
	}

	void context::wait_idle()
	{
		this->vk_render_context.wait_idle();
	}

}}


