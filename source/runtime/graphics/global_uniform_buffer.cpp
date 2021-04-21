#include "global_uniform_buffers.h"
#include "../core/camera.h"

namespace flower{ namespace graphics{

	global_uniform_buffers g_uniform_buffers = {};

	void global_uniform_buffers::update(uint32_t back_buffer_index)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float,std::chrono::seconds::period>(currentTime-startTime).count();

		global_matrix_vp ubo_vp{};

		ubo_vp.view = g_cam.get_view_matrix();
		ubo_vp.proj = g_cam.GetProjectMatrix(swaphchain->get_swapchain_extent().width,swaphchain->get_swapchain_extent().height);

		ubo_vps->map();
		ubo_vps->copy_to((void*)&ubo_vp,sizeof(ubo_vp));
		ubo_vps->unmap();
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
	}

	void global_uniform_buffers::release()
	{
		ubo_vps.reset();
	}

}}