#include "viking_room.h"
#include "graphics/vk/vk_buffer.h"
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
		// mesh_sponza.load_obj_mesh_new("data/model/sponza/sponza.obj");
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
		vkDestroyPipeline(device, render_pipeline, nullptr);
		vkDestroyPipelineLayout(device,pipeline_layout,nullptr);

		uniformBuffers.resize(0);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		mesh_texture.reset();
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		vertex_buffer.reset();
		index_buffer.reset();
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
		vkDestroyPipeline(device, render_pipeline, nullptr);
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
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

		ubo.view = scene_view_cam.get_view_matrix();
		ubo.proj = scene_view_cam.GetProjectMatrix(swapchain.get_swapchain_extent().width,swapchain.get_swapchain_extent().height);

		uniformBuffers[backBuffer_index]->map();
		uniformBuffers[backBuffer_index]->copy_to( (void*)&ubo,sizeof(ubo));
		uniformBuffers[backBuffer_index]->unmap();
	}

	void viking_room_scene::upload_vertex_buffer()
	{
		vertex_buffer = vk_vertex_buffer::create(&device,graphics_command_pool,mesh_data.vertices);
		index_buffer = vk_index_buffer::create(&device,graphics_command_pool,mesh_data.indices);
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
		mesh_texture = vk_texture::create_2d(&device,graphics_command_pool,"data/model/viking_room/viking_room.png");

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

			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pipeline);
			

			vertex_buffer->bind(cmd_buffer);
			index_buffer->bind(cmd_buffer);


			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptorSets[i], 0, nullptr);

			vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(mesh_data.indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(cmd_buffer);

			graphics_command_buffers[i]->end();
		}
	}

	void viking_room_scene::createGraphicsPipeline()
	{
		auto vertShaderCode = read_file_binary("data/model/viking_room/viking_room_vert.spv");
		auto fragShaderCode = read_file_binary("data/model/viking_room/viking_room_frag.spv");

		vk_shader_module vertShaderModule(device.device,vertShaderCode);
		vk_shader_module fragShaderModule(device.device,fragShaderCode);

		auto bindingDescription = vertex_buffer->get_binding_func();
		auto attributeDescriptions = vertex_buffer->get_attributes_func();

		// 顶点着色器填充
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		pipeline::shader_vertex_config(vertShaderStageInfo,vertShaderModule.get_handle());

		// 片元着色器填充
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		pipeline::shader_fragment_config(fragShaderStageInfo,fragShaderModule.get_handle());

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// 管线顶点输入信息
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// 组件信息
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		pipeline::input_assembly_config(inputAssembly);

		// 视口和裁剪区域
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
		VkPipelineViewportStateCreateInfo viewportState{};
		pipeline::viewport_config(viewportState,&viewport,&scissor);

		// 光栅化器配置
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		pipeline::rasterizer_config(rasterizer);

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisampling{};
		pipeline::msaa_config(multisampling);

		// 混合
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		pipeline::colorBlendAttachment_config(colorBlendAttachment);
		pipeline::colorBlend_config(colorBlending,&colorBlendAttachment);

		// 深度和模板测试
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		pipeline::depthStencil_config(depthStencil);


		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; 
		pipelineInfo.layout = pipeline_layout;
		pipelineInfo.renderPass = render_pass;
		pipelineInfo.subpass = 0; // sub Pass 索引
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; 
		pipelineInfo.basePipelineIndex = -1; 

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &render_pipeline) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建管线pipeline失败！");
		}
	}

	void viking_room_scene::createDescriptorPool()
	{
		const auto& swapchainnums = swapchain.get_imageViews().size();

		std::array<VkDescriptorPoolSize, 2> poolSizes{};

		// 第一种类型选择 uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapchainnums);

		// 第二种类型选择 combine_Image_sampler
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapchainnums);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapchainnums);

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

		std::vector<VkDescriptorSetLayout> layouts(swapchain_num, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain_num);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(swapchain_num);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("申请描述符集失败！");
		}

		for (size_t i = 0; i < swapchain_num; i++)
		{
			// 每个交换链图像都绑定对应的描述符集

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i]->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(uniform_buffer_mvp);

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &(mesh_texture->descriptor_info);

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

} }