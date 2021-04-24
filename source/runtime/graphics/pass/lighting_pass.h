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

	class lighting_pass : public vk_renderpass
	{
	public:
		operator VkRenderPass()
		{
			return render_pass;
		}

		lighting_pass(
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
			cmd_buf = vk_command_buffer::create(*in_mixdata.device,pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			create_renderpass();
			create_framebuffers();
		}

		~lighting_pass();

		static std::shared_ptr<lighting_pass> create(vk_renderpass_mix_data in_mixdata,VkCommandPool pool);

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
		VkSemaphore gbuffer_semaphore = VK_NULL_HANDLE;
		VkCommandPool pool;
	};

	// lighting ²ÄÖÊ
	class material_lighting: public material
	{
	public:
		material_lighting()
		{

		};

		~material_lighting()
		{
			
		}

		static std::shared_ptr<material_lighting> create(
			vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool,
			const std::vector<uint32_t>& in_texlib,
			glm::mat4 model_mat
		);
	};

}}