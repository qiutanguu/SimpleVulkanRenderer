#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/scene/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "graphics/vk/vk_texture.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/vk/vk_renderpass.h"
#include "graphics/vk/vk_device.h"

namespace flower { namespace graphics{

	class deferred_pass
	{
	public:
		operator VkRenderPass() { return render_pass; }

		deferred_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{
		}

		~deferred_pass();

		static std::shared_ptr<deferred_pass> create(vk_renderpass_mix_data in_mixdata);

	private:
		void create_framebuffers();
		void destroy_framebuffers();

		void create_renderpass();
		void destroy_renderpass();

	private:
		vk_renderpass_mix_data mix_data;

	public:
		VkRenderPass render_pass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> swapchain_framebuffers;
		std::vector<std::shared_ptr<vk_texture>> attach_colors;
		
	};

} }