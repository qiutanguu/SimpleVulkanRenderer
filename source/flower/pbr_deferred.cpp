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
		mesh_sponza.load_obj_mesh(&device,graphics_command_pool,"data/model/sponza/sponza.obj","");
		 
		vk_renderpass_mix_data mixdata(&device,swapchain);
		pass_deferred = deferred_pass::create(mixdata);  

		createGraphicsPipeline();
		upload_vertex_buffer();
		record_renderCommand();
	}

	void pbr_deferred::destroy_special()
	{
		pass_deferred.reset();

		pipeline_render.reset();

		mesh_sponza.sub_meshes.resize(0);

		sponza_vertex_buf.reset();
		sponza_index_buffer.resize(0);
	}

	void pbr_deferred::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		vk_renderpass_mix_data mixdata(&device,swapchain);
		pass_deferred = deferred_pass::create(mixdata);

		createGraphicsPipeline();

		record_renderCommand();
	}

	void pbr_deferred::cleanup_swapchain()
	{
		vk_runtime::cleanup_swapchain_default();

		pipeline_render.reset();
		texture_descriptor_sets.resize(0);
	}

	void pbr_deferred::update_before_commit(uint32_t backBuffer_index)
	{
		g_uniform_buffers.update(backBuffer_index);
	}

	void pbr_deferred::upload_vertex_buffer()
	{
		sponza_vertex_buf = vk_vertex_buffer::create(
			&device,
			graphics_command_pool,
			mesh_sponza.raw_data.pack_type_stream(g_shader_manager.texture_map_shader->per_vertex_attributes),
			g_shader_manager.texture_map_shader->per_vertex_attributes
		);

		for (auto& submesh : mesh_sponza.sub_meshes)
		{
			auto& indices = submesh.indices;
			sponza_index_buffer.push_back(vk_index_buffer::create(&device,graphics_command_pool,indices));
		}
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
			// renderPassInfo.renderPass = render_pass;
			// renderPassInfo.framebuffer = swapchain_framebuffers[i];

			renderPassInfo.renderPass = pass_deferred->render_pass;
			renderPassInfo.framebuffer = pass_deferred->swapchain_framebuffers[i];

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

			pipeline_render->bind(cmd_buffer);
			sponza_vertex_buf->bind(cmd_buffer);

			for (int32_t j = 0; j < mesh_sponza.sub_meshes.size(); j++)
			{
				auto index = i * mesh_sponza.sub_meshes.size() + j;
				vkCmdBindDescriptorSets(cmd_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_render->layout,0,1,texture_descriptor_sets[index]->descriptor_sets.data(),0,nullptr);

				auto& buf = sponza_index_buffer[j];
				buf->bind_and_draw(cmd_buffer);
			}

			vkCmdEndRenderPass(cmd_buffer);
			graphics_command_buffers[i]->end();
		}
	}

	void pbr_deferred::createGraphicsPipeline()
	{
		vk_pipeline_info pipe_info;

		pipe_info.vert_shader_module = g_shader_manager.texture_map_shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = g_shader_manager.texture_map_shader->frag_shader_module->handle;

		pipeline_render = vk_pipeline::create_by_shader(
			&device,
			VK_NULL_HANDLE,
			pipe_info,
			g_shader_manager.texture_map_shader,
			pass_deferred->render_pass 
		);

		const auto& swapchain_num = swapchain.get_imageViews().size();
		texture_descriptor_sets.resize(swapchain_num * mesh_sponza.sub_meshes.size());

		for (size_t i = 0; i < swapchain_num; i++)
		{
			for (size_t j = 0; j < mesh_sponza.sub_meshes.size(); j ++)
			{
				auto index = i * mesh_sponza.sub_meshes.size() + j;

				texture_descriptor_sets[index] = g_shader_manager.texture_map_shader->allocate_descriptor_set();
				texture_descriptor_sets[index]->set_buffer("ub_vp",g_uniform_buffers.ubo_vps[i]);
				texture_descriptor_sets[index]->set_buffer("ub_m",mesh_sponza.sub_meshes[j].buffer_ubo_model);
				texture_descriptor_sets[index]->set_image("base_color_texture",g_texture_manager.get_texture_vk( mesh_sponza.sub_meshes[j].material_using.map_diffuse));
			}
		}
	}

} }