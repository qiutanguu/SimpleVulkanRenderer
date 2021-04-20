#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_texture.h"
#include "vk/vk_swapchain.h"

namespace flower{ namespace graphics{

	struct global_matrix_vp
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class global_uniform_buffers
	{
	public:
		global_uniform_buffers(){  }

		void update(uint32_t back_buffer_index);
		void initialize(vk_device* in_device,vk_swapchain* in_swapchain,VkCommandPool in_pool);
		void release();

		global_matrix_vp vp;

		// view project matrix
		std::vector<std::shared_ptr<vk_buffer>> ubo_vps;

	private:
		vk_swapchain* swaphchain;
		vk_device* device;
		VkCommandPool pool;

		
		

	};

	extern global_uniform_buffers g_uniform_buffers;
}}