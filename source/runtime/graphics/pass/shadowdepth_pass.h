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

	class shadowdepth_pass : public vk_renderpass
	{
	public:
		operator VkRenderPass()
		{
			return render_pass;
		}

		shadowdepth_pass(
			vk_renderpass_mix_data in_mixdata
		): mix_data(in_mixdata)
		{
		
		}

		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) override
		{
			destroy_framebuffers();
			destroy_renderpass();

			mix_data = in_mixdata;
			cmd_buf = vk_command_buffer::create(*mix_data.device,mix_data.pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			create_renderpass();
			create_framebuffers();
		}

		~shadowdepth_pass();

		static std::shared_ptr<shadowdepth_pass> create(vk_renderpass_mix_data in_mixdata);

	private:
		void create_framebuffers();
		void destroy_framebuffers();

		void create_renderpass();
		void destroy_renderpass();

	private:
		vk_renderpass_mix_data mix_data;

	public:
		VkFramebuffer framebuffer;
		std::shared_ptr<vk_command_buffer> cmd_buf = nullptr;
		VkSemaphore shadowdepth_semaphore = VK_NULL_HANDLE;
	};


	class material_shadowdepth: public material
	{
	public:
		material_shadowdepth()
		{

		};

		~material_shadowdepth()
		{
		}

		static std::shared_ptr<material_shadowdepth> create(
			vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool,
			const std::vector<uint32_t>& in_texlib,
			glm::mat4 model_mat
		);
	};

}}