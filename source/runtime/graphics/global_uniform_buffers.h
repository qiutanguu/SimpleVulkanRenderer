#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_texture.h"
#include "vk/vk_swapchain.h"
#include "scene/light.h"

namespace flower{ namespace graphics{

	struct global_matrix_vp
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class global_uniform_buffers
	{
	public:
		global_uniform_buffers()
		{
			direct_light.color = glm::vec4(1.0f);
			direct_light.direction = glm::normalize(glm::vec4(-2.0f, 4.0f, -1.0f,0.0f));
			direct_light.shadow_mix = glm::vec4(8.0f,2.0f,1.0f,0);
		}

		void update(uint32_t back_buffer_index);
		void initialize(vk_device* in_device,vk_swapchain* in_swapchain,VkCommandPool in_pool);
		void release();

		directional_light direct_light;

		// view project matrix
		std::shared_ptr<vk_buffer> ubo_vps;
		std::shared_ptr<vk_buffer> ubo_directlight_vps;
		std::shared_ptr<vk_buffer> ubo_directional_light;
		
		float directional_light_theta = 89.0f;
		float directional_light_rotate_speed = 10.0f;

		void io_process(float dt)
		{
			directional_light_theta += dt * directional_light_rotate_speed;

			if(directional_light_theta > 360.0f)
			{
				directional_light_theta -= 360.0f;
			}

			else if( directional_light_theta < 0.0f)
			{
				directional_light_theta += 360.0f;
			}
		}

	private:
		vk_swapchain* swaphchain;
		vk_device* device;
		VkCommandPool pool;
	};

	extern global_uniform_buffers g_uniform_buffers;
}}