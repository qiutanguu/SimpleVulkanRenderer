#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "graphics/vk/vk_texture.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/vk/vk_device.h"

namespace flower { namespace graphics{

	class base_pass
	{
	public:
		operator VkRenderPass() { return render_pass; }

		base_pass(
			vk_device* device,
			vk_depth_resource& depth_resource,
			vk_swapchain& swapchain) : 
			device(device),
			depth_resource(depth_resource),
			swapchain(swapchain)
		{
			
		}

		~base_pass()
		{
			vkDestroyRenderPass(*device, render_pass, nullptr);
		}

		static std::shared_ptr<base_pass> create(
			vk_device* device,
			vk_depth_resource& depth_resource,
			vk_swapchain& swapchain);

	private:
		vk_device* device;
		vk_depth_resource& depth_resource;
		vk_swapchain& swapchain;

		VkRenderPass render_pass;
	};

} }