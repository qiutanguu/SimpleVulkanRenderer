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
		mesh_data.load_obj_mesh("data/model/viking_room/viking_room.obj");
		upload_vertex_buffer();

		createDescriptorSetLayout();

		createGraphicsPipeline();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
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
		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
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

	void viking_room_scene::createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.physical_device, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建纹理采样器失败！");
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

	void viking_room_scene::createTextureImageView()
	{
		textureImageView = create_imageView(&textureImage, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT,device);
	}

	void viking_room_scene::createTextureImage()
	{
		auto path = "data/model/viking_room/viking_room.png";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) 
		{
			LOG_IO_FATAL("加载图片{0}失败！",path);
		}

		auto stageBuffer = vk_buffer::create(
			device,
			graphics_command_pool,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			imageSize,
			pixels
		);

		stbi_image_free(pixels);

		create_image(
			texWidth, 
			texHeight, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			textureImage, 
			textureImageMemory,
			device
		);

		transition_image_layout(
			textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			graphics_command_pool,
			device,
			device.graphics_queue
		);

		copy_buffer_to_image(
			stageBuffer->buffer, 
			textureImage, 
			static_cast<uint32_t>(texWidth), 
			static_cast<uint32_t>(texHeight),
			graphics_command_pool,
			device,
			device.graphics_queue
		);

		transition_image_layout(
			textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			graphics_command_pool,
			device,
			device.graphics_queue
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

		auto vertShaderCode = read_file_binary("data/model/viking_room/viking_room_vert.spv");
		auto fragShaderCode = read_file_binary("data/model/viking_room/viking_room_frag.spv");

		vk_shader_module vertShaderModule(device.device,vertShaderCode);
		vk_shader_module fragShaderModule(device.device,fragShaderCode);

		auto bindingDescription = vertex_buffer->get_binding_func();
		auto attributeDescriptions = vertex_buffer->get_attributes_func();

		// 顶点着色器填充
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule.get_handle();
		vertShaderStageInfo.pName = shader_main_function_name::VS;

		// 片元着色器填充
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule.get_handle();
		fragShaderStageInfo.pName = shader_main_function_name::FS;

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
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

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
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// 光栅化器配置
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode =     VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; 
		rasterizer.depthBiasClamp = 0.0f; 
		rasterizer.depthBiasSlopeFactor = 0.0f; 

		// MSAA
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE; 
		multisampling.alphaToOneEnable = VK_FALSE;

		// 混合
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; 
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; 
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; 
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; 
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; 
		colorBlending.blendConstants[1] = 0.0f; 
		colorBlending.blendConstants[2] = 0.0f; 
		colorBlending.blendConstants[3] = 0.0f; 

		// Pipeline动态状态
		// 在视口大小改变等状态下，复用Pipeline而不是重新创建
		// 管道Layout （Uniform、 Sampler）等
		

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;

		// 深度和模板测试
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; 
		depthStencil.maxDepthBounds = 1.0f; 
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; 
		depthStencil.back = {}; 

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

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

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
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

} }