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

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		std::vector<VkPipelineStageFlags> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = wait_stages.data();

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.commandBufferCount = 1;

		// 1. gbuffer pass
		submitInfo.pWaitSemaphores = &semaphores_image_available[current_frame];
		submitInfo.pSignalSemaphores = &pass_gbuffer->gbuffer_semaphore;
		submitInfo.pCommandBuffers = &pass_gbuffer->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(pass_gbuffer->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		// 2. lighting pass
		submitInfo.pWaitSemaphores = &pass_gbuffer->gbuffer_semaphore;
		submitInfo.pSignalSemaphores = &pass_lighting->lighting_pass_semaphore;
		submitInfo.pCommandBuffers = &pass_lighting->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(pass_lighting->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		// 3. present pass
		submitInfo.pWaitSemaphores = &pass_lighting->lighting_pass_semaphore;
		submitInfo.pSignalSemaphores = &semaphores_render_finished[current_frame];
		submitInfo.pCommandBuffers = &graphics_command_buffers[back_buffer_index]->get_instance();
		vkResetFences(device,1,&inFlight_fences[current_frame]);
		vk_check(vkQueueSubmit(graphics_command_buffers[back_buffer_index]->get_queue(),1,&submitInfo,inFlight_fences[current_frame]));

		vk_runtime::present();
	}

	void pbr_deferred::initialize_special()
	{
		vk_renderpass_mix_data mixdata(&device,&swapchain,graphics_command_pool);
		pass_texture = texture_pass::create(mixdata);
		pass_gbuffer = gbuffer_pass::create(mixdata);
		pass_lighting = lighting_pass::create(mixdata);

		g_meshes_manager.sponza_mesh->register_renderpass(pass_texture,g_shader_manager.texture_map_shader);
		g_meshes_manager.sponza_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_shader);

		

		record_renderCommand();
	}

	void pbr_deferred::destroy_special()
	{
		pass_texture.reset();
		pass_gbuffer.reset();
		pass_lighting.reset();
	}

	void pbr_deferred::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		vk_renderpass_mix_data mixdata(&device,&swapchain,graphics_command_pool);
		pass_texture->swapchain_change(mixdata);
		pass_gbuffer->swapchain_change(mixdata);
		pass_lighting->swapchain_change(mixdata);

		// ÷ÿ–¬◊¢≤·renderpass
		g_meshes_manager.sponza_mesh->register_renderpass(pass_texture,g_shader_manager.texture_map_shader,false);
		g_meshes_manager.sponza_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_shader,false);

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
		pass_gbuffer->cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		{
			std::array<VkClearValue,4> clearValues;
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[3].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass =  pass_gbuffer->render_pass;
			renderPassBeginInfo.framebuffer = pass_gbuffer->framebuffer;
			renderPassBeginInfo.renderArea.offset = {0,0};
			renderPassBeginInfo.renderArea.extent = swapchain.get_swapchain_extent();
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(*pass_gbuffer->cmd_buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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
			vkCmdSetViewport(*pass_gbuffer->cmd_buf, 0, 1, &viewport);
			vkCmdSetScissor(*pass_gbuffer->cmd_buf, 0, 1, &scissor);

			g_meshes_manager.sponza_mesh->draw(pass_gbuffer->cmd_buf,renderpass_type::gbuffer_pass);

			vkCmdEndRenderPass(*pass_gbuffer->cmd_buf);
		}
		pass_gbuffer->cmd_buf->end();
		

		pass_lighting->cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		{
			std::array<VkClearValue,1> clearValues;
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass =  pass_lighting->render_pass;
			renderPassBeginInfo.framebuffer = pass_lighting->framebuffer;
			renderPassBeginInfo.renderArea.offset = {0,0};
			renderPassBeginInfo.renderArea.extent = swapchain.get_swapchain_extent();
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(*pass_lighting->cmd_buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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
			vkCmdSetViewport(*pass_lighting->cmd_buf, 0, 1, &viewport);
			vkCmdSetScissor(*pass_lighting->cmd_buf, 0, 1, &scissor);

			virtual_full_screen_triangle_draw(pass_lighting->cmd_buf,pass_lighting->lighting_material);

			vkCmdEndRenderPass(*pass_lighting->cmd_buf);
		}
		pass_lighting->cmd_buf->end();

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