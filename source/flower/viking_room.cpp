#include "viking_room.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/vk/vk_shader.h"
#include "core/core.h"

namespace flower{ namespace graphics{

	void viking_room_scene::config_before_init()
	{
		features.samplerAnisotropy = true;
	}

	void viking_room_scene::tick(float time, float delta_time)
	{
		uint32_t back_buffer_index = vk_runtime::acquire_next_present_image();

		update_before_commit(back_buffer_index);
		vk_runtime::submit(graphics_command_buffers[back_buffer_index]);
		vk_runtime::present();
	}

	void viking_room_scene::initialize_special()
	{
		mesh_data.load_obj_mesh_new("data/model/viking_room/viking_room.obj","");
		mesh_sponza.load_obj_mesh_new("data/model/sponza/sponza.obj","");
		upload_vertex_buffer();

		createDescriptorSetLayout();

		createGraphicsPipeline();
		createTextureImage();

		create_uniform_buffer();
		createDescriptorPool();
		createDescriptorSet();

		record_renderCommand();
	}
	
	void viking_room_scene::destroy_special()
	{
		pipeline_render.reset();

		vkDestroyPipelineLayout(device,pipeline_layout,nullptr);

		uniformBuffers.resize(0);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		mesh_texture.reset();
		sponza_textures.resize(0);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		sponza_vertex_buf.reset();
		sponza_index_buffer.resize(0);
	}

	void viking_room_scene::recreate_swapchain()
	{
		vk_runtime::recreate_swapchain_default();

		createGraphicsPipeline();
		create_uniform_buffer();
		createDescriptorPool();
		createDescriptorSet();

		record_renderCommand();
	}

	void viking_room_scene::cleanup_swapchain()
	{
		vk_runtime::cleanup_swapchain_default();

		pipeline_render.reset();

		vkDestroyPipelineLayout(device,pipeline_layout,nullptr);
		uniformBuffers.resize(0);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	void viking_room_scene::update_before_commit(uint32_t backBuffer_index)
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

	void viking_room_scene::upload_vertex_buffer()
	{
		sponza_vertex_buf = vk_vertex_buffer::create(&device,graphics_command_pool,mesh_sponza.vertices_data,mesh_sponza.vertices_attributes);
		for (auto& submesh : mesh_sponza.sub_meshes)
		{
			auto& indices = submesh.indices;
			sponza_index_buffer.push_back(vk_index_buffer::create(&device,graphics_command_pool,indices));
		}
	}

	void viking_room_scene::create_uniform_buffer()
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

	void viking_room_scene::createTextureImage()
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

	void viking_room_scene::record_renderCommand()
	{
		// 绘制命令
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

				vkCmdBindDescriptorSets(cmd_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_layout,0,1,&descriptorSets[index],0,nullptr);

				auto& buf = sponza_index_buffer[j];
				buf->bind_and_draw(cmd_buffer);
			}

			vkCmdEndRenderPass(cmd_buffer);
			graphics_command_buffers[i]->end();
		}
	}

	void viking_room_scene::createGraphicsPipeline()
	{
		auto vert_shader_module = vk_shader_module::create(&device,"data/model/viking_room/viking_room_vert.spv",VK_SHADER_STAGE_VERTEX_BIT);
		auto frag_shader_module = vk_shader_module::create(&device,"data/model/viking_room/viking_room_frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);

		auto bindings = sponza_vertex_buf->get_input_binding();
		auto attributes = sponza_vertex_buf->get_input_attribute(mesh_sponza.vertices_attributes);

		vk_pipeline_info pipe_info;
		pipe_info.vert_shader_module = vert_shader_module->handle;
		pipe_info.frag_shader_module = frag_shader_module->handle;

		pipeline_render = vk_pipeline::create_single_binding(&device,VK_NULL_HANDLE,pipe_info,bindings,attributes,pipeline_layout,render_pass);
	}

	void viking_room_scene::createDescriptorPool()
	{
		const auto& swapchainnums = swapchain.get_imageViews().size();

		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		// 第一种类型选择 uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1000u;// static_cast<uint32_t>(swapchainnums);

		// 第二种类型选择 combine_Image_sampler
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1000u; //static_cast<uint32_t>(swapchainnums);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 10000u; // static_cast<uint32_t>(swapchainnums);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建描述符池失败!");
		}
	}

	void viking_room_scene::createDescriptorSetLayout()
	{
		// 顶点着色器的UBO
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; 
		uboLayoutBinding.pImmutableSamplers = nullptr;

		// 片元着色器的Sampler
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// 组合
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建描述符集布局失败！");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&pipeline_layout)!=VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("创建管线布局失败！");
		}
	}

	void viking_room_scene::createDescriptorSet()
	{
		const auto& swapchain_num = swapchain.get_imageViews().size();

		std::vector<VkDescriptorSetLayout> layouts(swapchain_num * mesh_sponza.sub_meshes.size(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain_num * mesh_sponza.sub_meshes.size());
		allocInfo.pSetLayouts = layouts.data();

		// 为每个交换链每个sub mesh 添加一个描述符集
		descriptorSets.resize(swapchain_num * mesh_sponza.sub_meshes.size());
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("申请描述符集失败！");
		}

		for (size_t i = 0; i < swapchain_num; i++)
		{
			// 每个交换链图像都绑定对应的描述符集
			auto& texture = mesh_texture->descriptor_info;

			for (size_t j = 0; j < mesh_sponza.sub_meshes.size(); j ++)
			{
				auto index = i * mesh_sponza.sub_meshes.size() + j;

				if (mesh_sponza.sub_meshes[j].material_using.map_Kd != "")
				{
					texture = sponza_textures[j]->descriptor_info;
				}
				

				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[i]->buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(uniform_buffer_mvp);

				std::array<VkWriteDescriptorSet,2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[index];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = descriptorSets[index];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &texture;

				vkUpdateDescriptorSets(device,static_cast<uint32_t>(descriptorWrites.size()),descriptorWrites.data(),0,nullptr);
			}

			
		}
	}

} }