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
#include "graphics/vk/vk_command_buffer.h"

namespace flower { namespace graphics{

	class gbuffer_pass
	{
	public:
		operator VkRenderPass()
		{
			return render_pass;
		}

		gbuffer_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{
		}

		~gbuffer_pass();

		static std::shared_ptr<gbuffer_pass> create(vk_renderpass_mix_data in_mixdata);

	private:
		void create_framebuffers();
		void destroy_framebuffers();

		void create_renderpass();
		void destroy_renderpass();

	private:
		vk_renderpass_mix_data mix_data;
		std::shared_ptr<vk_command_buffer> cmd_buf = nullptr;
	public:
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkFramebuffer framebuffer;
	};

}}