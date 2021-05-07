#include "ui_overlay.h"
#include "imgui/imgui.h"
#include "../shader_manager.h"
#include "ui_style.h"

namespace flower{ namespace graphics{

	ui_overlay::ui_overlay()
	{
		ImGui::CreateContext();

		// 颜色配置
		ImGuiIO& io = ImGui::GetIO(); (void)io;			
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       

		SetupImGuiStyle(false,0.5f);
		ImGuiStyle& style = ImGui::GetStyle();
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w *= 0.2f;
		}
		
		io.FontGlobalScale = scale;
	}

	ui_overlay::~ui_overlay()
	{
		if(has_init)
		{
			freeResources();
		}
	}

	std::shared_ptr<ui_overlay> ui_overlay::create(vk_device* device,VkCommandPool in_pool,const VkRenderPass renderPass,const VkPipelineCache pipelineCache)
	{
		std::shared_ptr<ui_overlay> ret = std::make_shared<ui_overlay>();
		ret->initialize(device,in_pool,renderPass,pipelineCache);
		return ret;
	}

	void ui_overlay::on_swapchain_change(const VkRenderPass renderPass,const VkPipelineCache pipelineCache)
	{
		vkDestroyPipelineLayout(*device,pipelineLayout,nullptr);
		vkDestroyPipeline(*device,pipeline,nullptr);
		preparePipeline(renderPass,pipelineCache);
	}
	
	void ui_overlay::initialize(vk_device* in_device,VkCommandPool in_pool,const VkRenderPass renderPass,const VkPipelineCache pipelineCache)
	{
		device = in_device;
		queue = in_device->graphics_queue;
		pool = in_pool;

		auto loadShader = [&](std::shared_ptr<vk_shader_module> shader_module)
		{
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = shader_module->stage;
			shaderStage.module = shader_module->handle;
			shaderStage.pName = "main";
			return shaderStage;
		};

		ASSERT(g_shader_manager.ui_vertex_shader != nullptr, "ui在g_shader_manager前初始化！请调整顺序！");

		shaders = {
			loadShader(g_shader_manager.ui_vertex_shader),
			loadShader(g_shader_manager.ui_fragment_shader)
		};

		prepareResources();
		preparePipeline(renderPass,pipelineCache);
		has_init = true;
	}

	// 准备vulkan资源
	void ui_overlay::prepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		//创建字体纹理
		unsigned char* fontData;
		int texWidth,texHeight;

		const std::string filename = "data/font/Roboto-Medium.ttf";

		io.Fonts->AddFontFromFileTTF(filename.c_str(),16.0f);
		io.Fonts->GetTexDataAsRGBA32(&fontData,&texWidth,&texHeight);

		VkDeviceSize uploadSize = texWidth*texHeight*4*sizeof(char);

		// Create target image for copy
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vk_check(vkCreateImage(*device,&imageInfo,nullptr,&fontImage));
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(*device,fontImage,&memReqs);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->find_memory_type(memReqs.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vk_check(vkAllocateMemory(*device,&memAllocInfo,nullptr,&fontMemory));
		vk_check(vkBindImageMemory(*device,fontImage,fontMemory,0));

		// Image view
		VkImageViewCreateInfo viewInfo{ };
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = fontImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		vk_check(vkCreateImageView(*device,&viewInfo,nullptr,&fontView));

		// Staging buffers for font data upload
		std::shared_ptr<vk_buffer> stagingBuffer = vk_buffer::create(
			*device, pool,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uploadSize,fontData
		);

		transition_image_layout(
			fontImage, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			pool,
			*device,
			device->graphics_queue
		);

		// Copy
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = texWidth;
		bufferCopyRegion.imageExtent.height = texHeight;
		bufferCopyRegion.imageExtent.depth = 1;

		copy_buffer_to_image(
			stagingBuffer->buffer, 
			fontImage, 
			static_cast<uint32_t>(texWidth), 
			static_cast<uint32_t>(texHeight),
			pool,
			*device,
			device->graphics_queue
		);

		transition_image_layout(
			fontImage, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			pool,
			*device,
			device->graphics_queue
		);

		stagingBuffer.reset();

		// Font texture Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vk_check(vkCreateSampler(*device,&samplerInfo,nullptr,&sampler));

		auto descriptorPoolSize = [](
			VkDescriptorType type,
			uint32_t descriptorCount)
		{
			VkDescriptorPoolSize descriptorPoolSize {};
			descriptorPoolSize.type = type;
			descriptorPoolSize.descriptorCount = descriptorCount;
			return descriptorPoolSize;
		};

		// Descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes = 
		{
			descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2;

		vk_check(vkCreateDescriptorPool(*device,&descriptorPoolInfo,nullptr,&descriptorPool));


		auto descriptorSetLayoutBinding = [](
			VkDescriptorType type,
			VkShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount = 1)
		{
			VkDescriptorSetLayoutBinding setLayoutBinding {};
			setLayoutBinding.descriptorType = type;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorCount = descriptorCount;
			return setLayoutBinding;
		};

		// Descriptor set layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout {};
		descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayout.pBindings = setLayoutBindings.data();
		descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		vk_check(vkCreateDescriptorSetLayout(*device,&descriptorLayout,nullptr,&descriptorSetLayout));

		// Descriptor set
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		vk_check(vkAllocateDescriptorSets(*device,&allocInfo,&descriptorSet));

		VkDescriptorImageInfo fontDescriptor= {};
		fontDescriptor.sampler = sampler;
		fontDescriptor.imageView = fontView;
		fontDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		auto writeDescriptorSet = [](
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo *imageInfo,
			uint32_t descriptorCount = 1)
		{
			VkWriteDescriptorSet writeDescriptorSet {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = imageInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		};

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
		};
		vkUpdateDescriptorSets(*device,static_cast<uint32_t>(writeDescriptorSets.size()),writeDescriptorSets.data(),0,nullptr);
	}

	// 准备渲染管线
	void ui_overlay::preparePipeline(const VkRenderPass renderPass,const VkPipelineCache pipelineCache)
	{
		// Pipeline layout
		// Push constants for UI rendering parameters
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.size = sizeof(PushConstBlock);
		pushConstantRange.offset = 0;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
		vk_check(vkCreatePipelineLayout(*device,&pipelineLayoutCreateInfo,nullptr,&pipelineLayout));

		// Setup graphics pipeline for UI rendering
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Enable blending
		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = rasterizationSamples;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaders.size());
		pipelineCreateInfo.pStages = shaders.data();
		pipelineCreateInfo.subpass = subpass;

		auto vertexInputBindingDescription = [](
			uint32_t binding,
			uint32_t stride,
			VkVertexInputRate inputRate)
		{
			VkVertexInputBindingDescription vInputBindDescription {};
			vInputBindDescription.binding = binding;
			vInputBindDescription.stride = stride;
			vInputBindDescription.inputRate = inputRate;
			return vInputBindDescription;
		};

		// Vertex bindings an attributes based on ImGui vertex definition
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = 
		{
			vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
		};

		auto vertexInputAttributeDescription = [](
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset)
		{
			VkVertexInputAttributeDescription vInputAttribDescription {};
			vInputAttribDescription.location = location;
			vInputAttribDescription.binding = binding;
			vInputAttribDescription.format = format;
			vInputAttribDescription.offset = offset;
			return vInputAttribDescription;
		};

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
			vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
			vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
		};

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());

		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		vk_check(vkCreateGraphicsPipelines(*device,pipelineCache,1,&pipelineCreateInfo,nullptr,&pipeline));
	}


	bool ui_overlay::need_update()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		if(!imDrawData)
		{
			return false;
		};

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount*sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount*sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size
		if((vertexBufferSize==0)||(indexBufferSize==0))
		{
			return false;
		}

		// Vertex buffer
		if((vertexBuffer == nullptr) || (vertexCount!=imDrawData->TotalVtxCount))
		{
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount*sizeof(ImDrawIdx);
		if((indexBuffer == nullptr)||(indexCount<imDrawData->TotalIdxCount))
		{
			updateCmdBuffers = true;
		}

		return updateCmdBuffers;
	}


	// 更新顶点缓冲和次序缓冲
	bool ui_overlay::update()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffers = false;

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount*sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount*sizeof(ImDrawIdx);

		// Vertex buffer
		if((vertexBuffer == nullptr) || (vertexCount!=imDrawData->TotalVtxCount))
		{
			if(vertexBuffer!=nullptr)
			{
				vertexBuffer->unmap();
			}
			vertexBuffer.reset();

			vertexBuffer = vk_buffer::create(
				*device,pool,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				vertexBufferSize,
				nullptr
			);
			vertexCount = imDrawData->TotalVtxCount;
			vertexBuffer->unmap();
			vertexBuffer->map();
			updateCmdBuffers = true;
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount*sizeof(ImDrawIdx);
		if((indexBuffer == nullptr)||(indexCount<imDrawData->TotalIdxCount))
		{
			if(indexBuffer!=nullptr)
			{
				indexBuffer->unmap();
			}
			indexBuffer.reset();

			indexBuffer = vk_buffer::create(
				*device,pool,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				indexBufferSize,
				nullptr
			);

			indexCount = imDrawData->TotalIdxCount;
			indexBuffer->map();
			updateCmdBuffers = true;
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer->mapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer->mapped;

		for(int n = 0; n<imDrawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst,cmd_list->VtxBuffer.Data,cmd_list->VtxBuffer.Size*sizeof(ImDrawVert));
			memcpy(idxDst,cmd_list->IdxBuffer.Data,cmd_list->IdxBuffer.Size*sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		// Flush to make writes visible to GPU
		vertexBuffer->flush();
		indexBuffer->flush();

		return updateCmdBuffers;
	}

	void ui_overlay::draw(const VkCommandBuffer commandBuffer)
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if((!imDrawData)||(imDrawData->CmdListsCount==0))
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);
		vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,&descriptorSet,0,NULL);

		pushConstBlock.scale = glm::vec2(2.0f/io.DisplaySize.x,2.0f/io.DisplaySize.y);
		pushConstBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(commandBuffer,pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConstBlock),&pushConstBlock);

		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(commandBuffer,0,1,&vertexBuffer->buffer,offsets);
		vkCmdBindIndexBuffer(commandBuffer,indexBuffer->buffer,0,VK_INDEX_TYPE_UINT16);

		for(int32_t i = 0; i<imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for(int32_t j = 0; j<cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x),0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y),0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z-pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w-pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer,0,1,&scissorRect);
				vkCmdDrawIndexed(commandBuffer,pcmd->ElemCount,1,indexOffset,vertexOffset,0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	void ui_overlay::resize(uint32_t width,uint32_t height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width),(float)(height));
	}

	// 释放资源
	void ui_overlay::freeResources()
	{
		ImGui::DestroyContext();
		vertexBuffer.reset();
		indexBuffer.reset();
		vkDestroyImageView(*device,fontView,nullptr);
		vkDestroyImage(*device,fontImage,nullptr);
		vkFreeMemory(*device,fontMemory,nullptr);
		vkDestroySampler(*device,sampler,nullptr);
		vkDestroyDescriptorSetLayout(*device,descriptorSetLayout,nullptr);
		vkDestroyDescriptorPool(*device,descriptorPool,nullptr);
		vkDestroyPipelineLayout(*device,pipelineLayout,nullptr);
		vkDestroyPipeline(*device,pipeline,nullptr);
	}
} }