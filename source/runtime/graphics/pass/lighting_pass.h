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

	class present_pass : public vk_renderpass
	{
	public:
		operator VkRenderPass() { return render_pass; }

		present_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{
		}

		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) override
		{
			destroy_framebuffers();
			destroy_renderpass();
			mix_data = in_mixdata;
			create_renderpass();
			create_framebuffers();
		}

		~present_pass();

		static std::shared_ptr<present_pass> create(vk_renderpass_mix_data in_mixdata);

	private:
		void create_framebuffers();
		void destroy_framebuffers();

		void create_renderpass();
		void destroy_renderpass();

	private:
		vk_renderpass_mix_data mix_data;

	public:
		std::vector<VkFramebuffer> swapchain_framebuffers;
		std::vector<std::shared_ptr<vk_texture>> attach_colors;
	};

} }