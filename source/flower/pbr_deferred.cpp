#include "pbr_deferred.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/global_uniform_buffers.h"
#include "core/core.h"
#include "graphics/shader_manager.h"

namespace flower{ namespace graphics{

	void pbr_deferred::config_before_init()
	{
		features.samplerAnisotropy = true;
	}

	void pbr_deferred::tick(float time, float delta_time)
	{
		uint32_t back_buffer_index = vk_runtime::acquire_next_present_image();

		update_before_commit(back_buffer_index);
		vk_runtime::submit(graphics_command_buffers[back_buffer_index]);
		vk_runtime::present();
	}

	void pbr_deferred::initialize_special()
	{
		vk_renderpass_mix_data mixdata(&device,&swapchain);
		pass_texture = texture_pass::create(mixdata);
		pass_gbuffer = gbuffer_pass::create(mixdata);

		g_meshes_manager.sponza_mesh->register_renderpass(renderpass_type::texture_pass,pass_texture->render_pass);

		record_renderCommand();
	}

	void pbr_deferred::destroy_special()
	{
		pass_texture.reset();
		pass_gbuffer.reset();
	}

	void pbr_deferred::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		vk_renderpass_mix_data mixdata(&device,&swapchain);
		pass_texture->swapchain_change(mixdata);
		pass_gbuffer->swapchain_change(mixdata);

		// ÷ÿ–¬◊¢≤·renderpass
		g_meshes_manager.sponza_mesh->register_renderpass(renderpass_type::texture_pass,pass_texture->render_pass,false);

		record_renderCommand();
	}

	void pbr_deferred::cleanup_swapchain()
	{
		vk_runtime::cleanup_swapchain_default();
	}

	void pbr_deferred::update_before_commit(uint32_t backBuffer_index)
	{
		g_uniform_buffers.update(backBuffer_index);
	}


	void pbr_deferred::record_renderCommand()
	{
		// ªÊ÷∆√¸¡Ó
		for (size_t i = 0; i < graphics_command_buffers.size(); i++) 
		{
			graphics_command_buffers[i]->begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

			auto& cmd_buffer = graphics_command_buffers[i]->get_instance();
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

			renderPassInfo.renderPass = pass_texture->render_pass;
			renderPassInfo.framebuffer = pass_texture->swapchain_framebuffers[i];

			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapchain.get_swapchain_extent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(cmd_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float) swapchain.get_swapchain_extent().width;
			viewport.height = (float) swapchain.get_swapchain_extent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = swapchain.get_swapchain_extent();

			vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
			vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

			g_meshes_manager.sponza_mesh->draw(graphics_command_buffers[i],renderpass_type::texture_pass);

			vkCmdEndRenderPass(cmd_buffer);
			graphics_command_buffers[i]->end();
		}
	}

} }