#include "pbr_deferred.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/global_uniform_buffers.h"
#include "core/core.h"
#include "graphics/shader_manager.h"
#include "graphics/ui/imgui/imgui.h"
#include "core/input.h"

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

		// 0. shadow depth pass
		submitInfo.pWaitSemaphores = &semaphores_image_available[current_frame];
		submitInfo.pSignalSemaphores = &pass_shadowdepth->shadowdepth_semaphore;
		submitInfo.pCommandBuffers = &pass_shadowdepth->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(pass_shadowdepth->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		// 1. gbuffer pass
		submitInfo.pWaitSemaphores = &pass_shadowdepth->shadowdepth_semaphore;
		submitInfo.pSignalSemaphores = &pass_gbuffer->gbuffer_semaphore;
		submitInfo.pCommandBuffers = &pass_gbuffer->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(pass_gbuffer->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		submitInfo.pWaitSemaphores = &pass_gbuffer->gbuffer_semaphore;
		submitInfo.pSignalSemaphores = &compute_edge_detect_pass->semaphore;
		submitInfo.pCommandBuffers = &compute_edge_detect_pass->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(compute_edge_detect_pass->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		// 2. lighting pass
		submitInfo.pWaitSemaphores = &compute_edge_detect_pass->semaphore;
		submitInfo.pSignalSemaphores = &pass_lighting->lighting_pass_semaphore;
		submitInfo.pCommandBuffers = &pass_lighting->cmd_buf->get_instance();
		vk_check(vkQueueSubmit(pass_lighting->cmd_buf->get_queue(),1,&submitInfo,VK_NULL_HANDLE));

		// 3. tonemapper ui present pass
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

		pass_shadowdepth = shadowdepth_pass::create(mixdata);
		pass_gbuffer = gbuffer_pass::create(mixdata);
		pass_lighting = lighting_pass::create(mixdata);
		pass_tonemapper = tonemapper_pass::create(mixdata);

		g_meshes_manager.sponza_mesh->register_renderpass(pass_shadowdepth,g_shader_manager.shadowdepth_shader,renderpass_type::shadowdepth_pass);
		g_meshes_manager.miku_mesh->register_renderpass(pass_shadowdepth,g_shader_manager.shadowdepth_shader,renderpass_type::shadowdepth_pass);

		g_meshes_manager.sponza_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_shader,renderpass_type::gbuffer_pass);
		g_meshes_manager.miku_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_character_shader,renderpass_type::gbuffer_character_pass);

		compute_edge_detect_pass = compute_edge_detect::create(&device,compute_command_pool);

		// uiע�������һ��renderpass��
		ui_context = ui_overlay::create(&device,graphics_command_pool,pass_tonemapper->render_pass);
		record_renderCommand();
	}

	void pbr_deferred::destroy_special()
	{
		pass_shadowdepth.reset();
		pass_gbuffer.reset();
		pass_lighting.reset();
		pass_tonemapper.reset();
		ui_context.reset();
		compute_edge_detect_pass.reset();
	}

	void pbr_deferred::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		vk_renderpass_mix_data mixdata(&device,&swapchain,graphics_command_pool);
		pass_shadowdepth->swapchain_change(mixdata);
		pass_gbuffer->swapchain_change(mixdata);
		pass_lighting->swapchain_change(mixdata);
		pass_tonemapper->swapchain_change(mixdata);
		ui_context->on_swapchain_change(pass_tonemapper->render_pass);

		// ����ע��renderpass
		g_meshes_manager.sponza_mesh->register_renderpass(pass_shadowdepth,g_shader_manager.shadowdepth_shader,renderpass_type::shadowdepth_pass,false);
		g_meshes_manager.miku_mesh->register_renderpass(pass_shadowdepth,g_shader_manager.shadowdepth_shader,renderpass_type::shadowdepth_pass,false);

		g_meshes_manager.sponza_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_shader,renderpass_type::gbuffer_pass,false);
		g_meshes_manager.sponza_mesh->register_renderpass(pass_gbuffer,g_shader_manager.gbuffer_character_shader,renderpass_type::gbuffer_character_pass,false);
		record_renderCommand();

		ui_context->updated = true;
	}

	void pbr_deferred::cleanup_swapchain()
	{
		vk_runtime::cleanup_swapchain_default();
	}

	void pbr_deferred::update_before_commit(uint32_t backBuffer_index)
	{
		g_uniform_buffers.update(backBuffer_index);
		ui_overlay_update();
	}

	void pbr_deferred::record_renderCommand()
	{
		gbuffer_record_command();
		lighting_record_command();
		shadowdepth_record_command();
		tonemapper_ui_record_command();

		compute_edge_detect_pass->build_command_buffer();
	}

	void pbr_deferred::gbuffer_record_command()
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
			g_meshes_manager.miku_mesh->draw(pass_gbuffer->cmd_buf,renderpass_type::gbuffer_character_pass);

			vkCmdEndRenderPass(*pass_gbuffer->cmd_buf);
		} 
		pass_gbuffer->cmd_buf->end();
	}

	void pbr_deferred::lighting_record_command()
	{
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
	}

	void pbr_deferred::shadowdepth_record_command()
	{
		pass_shadowdepth->cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		{
			std::array<VkClearValue,1> clearValues;
			clearValues[0].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass =  pass_shadowdepth->render_pass;
			renderPassBeginInfo.framebuffer = pass_shadowdepth->framebuffer;
			renderPassBeginInfo.renderArea.offset = {0,0};
			renderPassBeginInfo.renderArea.extent = g_scene_textures.scene_shadowdepth->get_extent_2d();
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(*pass_shadowdepth->cmd_buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float) g_scene_textures.scene_shadowdepth->width;
			viewport.height = (float) g_scene_textures.scene_shadowdepth->height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = g_scene_textures.scene_shadowdepth->get_extent_2d();
			vkCmdSetViewport(*pass_shadowdepth->cmd_buf, 0, 1, &viewport);
			vkCmdSetScissor(*pass_shadowdepth->cmd_buf, 0, 1, &scissor);

			static const float depthBiasConstant = 1.25f;
			static const float depthBiasSlope = 1.75f;

			vkCmdSetDepthBias(
				*pass_shadowdepth->cmd_buf,
				depthBiasConstant,
				0.0f,
				depthBiasSlope
			);

			g_meshes_manager.sponza_mesh->draw(pass_shadowdepth->cmd_buf,renderpass_type::shadowdepth_pass);
			g_meshes_manager.miku_mesh->draw(pass_shadowdepth->cmd_buf,renderpass_type::shadowdepth_pass);
			vkCmdEndRenderPass(*pass_shadowdepth->cmd_buf);
		}

		pass_shadowdepth->cmd_buf->end();
	}

	// tonemapper һ�������һ��pass�����˳���������¼ui����
	void pbr_deferred::tonemapper_ui_record_command()
	{
		for (size_t i = 0; i < graphics_command_buffers.size(); i++) 
		{
			graphics_command_buffers[i]->begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
			auto& cmd_buffer = graphics_command_buffers[i]->get_instance();

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = pass_tonemapper->render_pass;
			renderPassInfo.framebuffer = pass_tonemapper->swapchain_framebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapchain.get_swapchain_extent();
			std::array<VkClearValue, 1> clearValues{};
			clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
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

			virtual_full_screen_triangle_draw(graphics_command_buffers[i],pass_tonemapper->tonemapper_material);

			// NOTE: �����һ��Pass�������ui draw
			ui_context->draw(graphics_command_buffers[i]->get_instance());

			vkCmdEndRenderPass(cmd_buffer);
			graphics_command_buffers[i]->end();
		}
	}

	void pbr_deferred::ui_overlay_update()
	{
		ImGuiIO& io = ImGui::GetIO();

		// overlayһ���ǵ��ӵ�scene color�ϵ�
		io.DisplaySize = ImVec2(
			(float)swapchain.get_swapchain_extent().width, 
			(float)swapchain.get_swapchain_extent().height
		);

		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float,std::chrono::seconds::period>(
			currentTime - startTime).count();

		// ��ʱ�Ե�delta time��ʱ
		io.DeltaTime = time;
		startTime = currentTime;

		io.MousePos = ImVec2(input::current_mouse_pos.x, input::current_mouse_pos.y);
		io.MouseDown[0] = input::left_mouse_button_down;
		io.MouseDown[1] = input::right_mouse_button_down;

		// No better but simple
		for (size_t i = 0; i < graphics_command_buffers.size(); i++) 
		{
			vkWaitForFences(device,1,&inFlight_fences[i],VK_TRUE,UINT64_MAX);
		}

		// ����ui
		ImGui::NewFrame();
		{
			ui_layout();
		}
		ImGui::Render();

		if (ui_context->need_update() || ui_context->updated) 
		{
			ui_context->update();
			tonemapper_ui_record_command();

			// NOTE: Ӧ��ʹ��lazy update�ķ�ʽ
			// ui_context->updated = false;
			ui_context->updated = true;
		}
	}

	void pbr_deferred::ui_layout()
	{
		static bool show_base_color = true;
		static bool first_open = true;

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::Begin("flower config", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);  
		{
			ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::PushItemWidth(110.0f * ui_context->scale);
			{
				ImGui::SliderFloat("pcss dilation", &g_uniform_buffers.direct_light.shadow_mix.x, 0.0f, 100.0f); 
				ImGui::SliderFloat("pcf  dilation", &g_uniform_buffers.direct_light.shadow_mix.y, 0.0f, 100.0f); 
				ImGui::SliderFloat("switch       ", &g_uniform_buffers.direct_light.shadow_mix.z, -100.0f, 100.0f); 
				ImGui::Checkbox("show base color", &show_base_color);

				if(!first_open)
				{
					if(show_base_color)
					{
						pass_tonemapper->tonemapper_material->descriptor_set->set_image(
							"scenecolor_tex",
							g_scene_textures.normal_worldspace
						);
					}
					else
					{
						pass_tonemapper->tonemapper_material->descriptor_set->set_image(
							"scenecolor_tex",
							g_scene_textures.scene_color
						);
					}
				}

				ImGui::Checkbox("Play Animation", &start_play);
				if(start_play)
				{
					// ���Ŷ���

				}
			}
			ImGui::PopItemWidth();
		}
		ImGui::End();

		first_open = false;
	}

} }