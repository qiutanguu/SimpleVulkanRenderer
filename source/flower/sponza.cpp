#include "sponza.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/vk/vk_shader.h"
#include "core/core.h"

namespace flower{ namespace graphics{

	void sponza::config_before_init()
	{
		features.samplerAnisotropy = true;
	}

	void sponza::tick(float time, float delta_time)
	{
		uint32_t back_buffer_index = vk_runtime::acquire_next_present_image();

		update_before_commit(back_buffer_index);
		vk_runtime::submit(graphics_command_buffers[back_buffer_index]);
		vk_runtime::present();
	}

	void sponza::initialize_special()
	{
		mesh_sponza.load_obj_mesh_new("data/model/sponza/sponza.obj","");

		upload_vertex_buffer();
		createTextureImage();

		create_uniform_buffer();
		createGraphicsPipeline();

		record_renderCommand();
	}
	
	void sponza::destroy_special()
	{
		pipeline_render.reset();


		uniformBuffers.resize(0);

		mesh_texture.reset();
		sponza_textures.resize(0);

		sponza_vertex_buf.reset();
		sponza_index_buffer.resize(0);

		texture_shader_mix.reset();
	}

	void sponza::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		create_uniform_buffer();
		createGraphicsPipeline();
		
		record_renderCommand();
	}

	void sponza::cleanup_swapchain()
	{
		vk_runtime::cleanup_swapchain_default();

		pipeline_render.reset();
		texture_descriptor_sets.resize(0);
		uniformBuffers.resize(0);
	}

	void sponza::update_before_commit(uint32_t backBuffer_index)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		uniform_buffer_mvp ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

		ubo.view = scene_view_cam.get_view_matrix();
		ubo.proj = scene_view_cam.GetProjectMatrix(swapchain.get_swapchain_extent().width,swapchain.get_swapchain_extent().height);

		uniformBuffers[backBuffer_index]->map();
		uniformBuffers[backBuffer_index]->copy_to( (void*)&ubo,sizeof(ubo));
		uniformBuffers[backBuffer_index]->unmap();
	}

	void sponza::upload_vertex_buffer()
	{
		sponza_vertex_buf = vk_vertex_buffer::create(&device,graphics_command_pool,mesh_sponza.vertices_data,mesh_sponza.vertices_attributes);
		for (auto& submesh : mesh_sponza.sub_meshes)
		{
			auto& indices = submesh.indices;
			sponza_index_buffer.push_back(vk_index_buffer::create(&device,graphics_command_pool,indices));
		}
	}

	void sponza::create_uniform_buffer()
	{
		VkDeviceSize bufferSize = sizeof(uniform_buffer_mvp);

		uniformBuffers.resize(swapchain.get_imageViews().size());

		for (size_t i = 0; i < uniformBuffers.size(); i++) 
		{
			auto buffer = vk_buffer::create(
				device,
				graphics_command_pool,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				nullptr
			);

			uniformBuffers[i] = buffer;
		}
	}

	void sponza::createTextureImage()
	{
		sponza_textures.resize(mesh_sponza.sub_meshes.size());
		
		for (size_t i = 0; i < mesh_sponza.sub_meshes.size(); i ++)
		{
			std::string pathpad = "data/model/sponza/";

			if(mesh_sponza.sub_meshes[i].material_using.map_Kd_set && mesh_sponza.sub_meshes[i].material_using.map_Kd!= "")
			{
				std::string combinePath = pathpad + mesh_sponza.sub_meshes[i].material_using.map_Kd;

				sponza_textures[i] = vk_texture::create_2d(&device,graphics_command_pool,VK_FORMAT_R8G8B8A8_SRGB,combinePath);

				sponza_textures[i]->update_sampler(
					VK_FILTER_LINEAR,
					VK_FILTER_LINEAR,
					VK_SAMPLER_MIPMAP_MODE_LINEAR,
					VK_SAMPLER_ADDRESS_MODE_REPEAT,
					VK_SAMPLER_ADDRESS_MODE_REPEAT,
					VK_SAMPLER_ADDRESS_MODE_REPEAT
				);
			}
		}
		
		mesh_texture = vk_texture::create_2d(&device,graphics_command_pool,VK_FORMAT_R8G8B8A8_SRGB,"data/image/checkerboard.png");

		mesh_texture->update_sampler(
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT
		);
	}

	void sponza::record_renderCommand()
	{
		// ªÊ÷∆√¸¡Ó
		for (size_t i = 0; i < graphics_command_buffers.size(); i++) 
		{
			graphics_command_buffers[i]->begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);

			auto& cmd_buffer = graphics_command_buffers[i]->get_instance();
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = render_pass;
			renderPassInfo.framebuffer = swapchain_framebuffers[i];
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

	void sponza::createGraphicsPipeline()
	{
		texture_shader_mix = vk_shader_mix::create(&device,false,"data/model/sponza/vert.spv","data/model/sponza/frag.spv");

		auto bindings = sponza_vertex_buf->get_input_binding();
		auto attributes = sponza_vertex_buf->get_input_attribute(mesh_sponza.vertices_attributes);

		vk_pipeline_info pipe_info;
		pipe_info.vert_shader_module = texture_shader_mix->vert_shader_module->handle;
		pipe_info.frag_shader_module = texture_shader_mix->frag_shader_module->handle;

		pipeline_render = vk_pipeline::create_single_binding(&device,VK_NULL_HANDLE,pipe_info,bindings,attributes,texture_shader_mix->pipeline_layout,render_pass);

		const auto& swapchain_num = swapchain.get_imageViews().size();
		texture_descriptor_sets.resize(swapchain_num * mesh_sponza.sub_meshes.size());

		auto& texture = mesh_texture;
		for (size_t i = 0; i < swapchain_num; i++)
		{
			for (size_t j = 0; j < mesh_sponza.sub_meshes.size(); j ++)
			{
				if (mesh_sponza.sub_meshes[j].material_using.map_Kd != "")
				{
					texture = sponza_textures[j];
				}

				auto index = i * mesh_sponza.sub_meshes.size() + j;
				texture_descriptor_sets[index] = texture_shader_mix->allocate_descriptor_set();
				texture_descriptor_sets[index]->set_buffer("ubo",uniformBuffers[i]);
				texture_descriptor_sets[index]->set_image("texSampler",texture);
			}
		}
	}

} }