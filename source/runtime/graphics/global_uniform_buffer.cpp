#include "global_uniform_buffers.h"
#include "../core/camera.h"

namespace flower{ namespace graphics{

	global_uniform_buffers g_uniform_buffers = {};
	glm::mat4 global_ident_mat4_model =  glm::rotate(glm::mat4(1.0f),glm::radians(0.0f),glm::vec3(-1.0f,0.0f,0.0f));

	void global_uniform_buffers::update(uint32_t back_buffer_index)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float,std::chrono::seconds::period>(currentTime-startTime).count();

		global_matrix_vp ubo_vp{};
		global_matrix_vp ubo_directlight_vp{};

		ubo_vp.view = g_cam.get_view_matrix();
		ubo_vp.proj = g_cam.GetProjectMatrix(swaphchain->get_swapchain_extent().width,swaphchain->get_swapchain_extent().height);

		ubo_vps->map();
		ubo_vps->copy_to((void*)&ubo_vp,sizeof(ubo_vp));
		ubo_vps->unmap();

		// 主光源更新
		float degree = directional_light_theta;

		direct_light.direction.x = 0.1f;
		direct_light.direction.z = cos(glm::radians(degree)); 
		direct_light.direction.y = sin(glm::radians(degree)); 
		direct_light.direction.w = 0.0f;

		direct_light.direction = normalize(direct_light.direction);

		// TODO: 
		// sponza bounds
		std::array<glm::vec3,8> bound_points = {
			glm::vec3(-2113.04f,-139.087f,-1301.09f), // 0
			glm::vec3(-2113.04f,-139.087f, 1215.97f), // 3
			glm::vec3(-2113.04f,1572.38f,-1301.09f),  // 4
			glm::vec3(-2113.04f,1572.38f,1215.97f),   // 7
			glm::vec3(1979.9f,-139.087f,-1301.09f),   // 1
			glm::vec3(1979.9f,-139.087f,1215.97f),    // 2
			glm::vec3(1979.9f,1572.38f,-1301.09f),    // 5
			glm::vec3(1979.9f,1572.38f,1215.97f)      // 6
		};

		glm::vec3 scene_center = glm::vec3(0);

		ubo_directlight_vp.view = glm::lookAt(glm::vec3(direct_light.direction), scene_center, glm::vec3(0, 1.0f, 0.0f));

		float min_x = std::numeric_limits<float>::max();
		float max_x = -std::numeric_limits<float>::max();

		float min_y = std::numeric_limits<float>::max();
		float max_y = -std::numeric_limits<float>::max();

		float min_z = std::numeric_limits<float>::max();
		float max_z = -std::numeric_limits<float>::max();

		// global ident model
		auto model = global_ident_mat4_model;

		for (auto& point : bound_points)
		{
			point = ubo_directlight_vp.view * (model * glm::vec4(point,1.0f));

			if(point.x<min_x)
			{
				min_x = point.x;
			}
			if(point.x>max_x)
			{
				max_x = point.x;
			}
			if(point.y<min_y)
			{
				min_y = point.y;
			}
			if(point.y>max_y)
			{
				max_y = point.y;
			}
			if(point.z<min_z)
			{
				min_z = point.z;
			}
			if(point.z>max_z)
			{
				max_z = point.z;
			}
		}

		ubo_directlight_vp.proj =  glm::ortho(min_x, max_x, min_y, max_y,-max_z,-min_z);

		// 灯光视图投影矩阵
		ubo_directlight_vps->map();
		ubo_directlight_vps->copy_to((void*)&ubo_directlight_vp,sizeof(ubo_directlight_vp));
		ubo_directlight_vps->unmap();

		// 灯光方向信息
		ubo_directional_light->map();
		ubo_directional_light->copy_to((void*)&direct_light,sizeof(ubo_vp));
		ubo_directional_light->unmap();
	}

	void global_uniform_buffers::initialize(vk_device* in_device,vk_swapchain* in_swapchain,VkCommandPool in_pool)
	{
		device = in_device;
		swaphchain = in_swapchain;
		pool = in_pool;

		VkDeviceSize bufferSize = sizeof(global_matrix_vp);

		ubo_vps = vk_buffer::create(
			*device,
			pool,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			nullptr
		);

		ubo_directlight_vps = vk_buffer::create(
			*device,
			pool,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			nullptr
		);

		bufferSize = sizeof(directional_light);
		ubo_directional_light = vk_buffer::create(
			*device,
			pool,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			nullptr
		);
	}


	void global_uniform_buffers::release()
	{
		ubo_vps.reset();
		ubo_directional_light.reset();
		ubo_directlight_vps.reset();
	}

}}