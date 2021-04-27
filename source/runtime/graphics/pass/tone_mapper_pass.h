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

namespace flower{namespace graphics{

	class material_tonemapper: public material
	{
	public:
		material_tonemapper()
		{

		};

		~material_tonemapper()
		{

		}

		static std::shared_ptr<material_tonemapper> create(
			vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool
		);
	};

	class tonemapper_pass: public vk_renderpass
	{
	public:
		operator VkRenderPass()
		{
			return render_pass;
		}

		tonemapper_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{
			type = renderpass_type::lighting_pass;
		}

		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) override
		{
			destroy_framebuffers();
			destroy_renderpass();

			mix_data = in_mixdata;
			create_renderpass();
			create_framebuffers();

			tonemapper_material.reset();
			tonemapper_material = material_tonemapper::create(in_mixdata.device,render_pass,in_mixdata.pool);
		}

		std::shared_ptr<material_tonemapper> tonemapper_material;

		~tonemapper_pass();

		static std::shared_ptr<tonemapper_pass> create(vk_renderpass_mix_data in_mixdata);

	private:
		void create_framebuffers();
		void destroy_framebuffers();

		void create_renderpass();
		void destroy_renderpass();

	private:
		vk_renderpass_mix_data mix_data;

	public:
		std::vector<VkFramebuffer> swapchain_framebuffers;
	};


}}