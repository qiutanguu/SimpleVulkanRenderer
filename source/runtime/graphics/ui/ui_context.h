#pragma once
#include "../vk/vk_device.h"
#include "../vk/vk_common.h"

namespace flower { namespace graphics{
	
	class ui_context
	{
	public:
		ui_context(
			GLFWwindow* inWindow,
			vk_device inDevice,
			VkSurfaceKHR inSurface,
			VkInstance inInstance,
			VkPhysicalDevice inGpu) : 
			window(inWindow),
			device(inDevice),
			surface(inSurface),
			instance(inInstance),
			gpu(inGpu)
		{
			
		}
		~ui_context() { }

		void initialize();
		void destroy();
	


	private:
		VkDescriptorPool descriptor_pool;
		VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
	private:
		void create_descriptor_pool()
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;

			vk_check(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool));
		}

		void destroy_descriptor_pool()
		{
			vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
		}
		
	private:
		GLFWwindow* window;
		VkInstance instance;
		VkPhysicalDevice gpu;
		vk_device& device;

		VkSurfaceKHR surface;


	};

}}