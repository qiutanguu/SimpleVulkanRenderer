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

	class texture_pass : public vk_renderpass
	{
	public:
		operator VkRenderPass() { return render_pass; }

		texture_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{

		}

		~texture_pass();

		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) override
		{
			destroy_framebuffers();
			destroy_renderpass();

			mix_data = in_mixdata;

			create_renderpass();
			create_framebuffers();
		}

		static std::shared_ptr<texture_pass> create(vk_renderpass_mix_data in_mixdata);

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

	// 仅采样basecolor纹理并显示的材质
	class material_texture: public material
	{
	public:
		material_texture(){ };
		~material_texture(){ }

		static std::shared_ptr<material_texture> create(
			vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool,
			uint32_t map_colortex,
			glm::mat4 model_mat
		);
	};

} }