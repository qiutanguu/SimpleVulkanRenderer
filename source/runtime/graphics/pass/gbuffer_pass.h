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

	class gbuffer_pass : public vk_renderpass
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
			type = renderpass_type::gbuffer_pass;
		}

		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) override
		{
			destroy_framebuffers();
			destroy_renderpass();

			mix_data = in_mixdata;

			create_renderpass();
			create_framebuffers();
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
		VkFramebuffer framebuffer;
	};

	// gbuffer pbr ²ÄÖÊ
	class material_gbuffer: public material
	{
	public:
		material_gbuffer()
		{
		};
		~material_gbuffer()
		{
		}

		static std::shared_ptr<material_gbuffer> create(
			vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool,
			const std::vector<uint32_t>& in_texlib,
			glm::mat4 model_mat
		);
	};

}}