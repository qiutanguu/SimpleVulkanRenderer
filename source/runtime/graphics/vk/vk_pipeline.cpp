#include "vk_pipeline.h"

namespace flower { namespace graphics{

	vk_pipeline_info::vk_pipeline_info()
	{
		input_assembly_state = {};
		input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		rasterization_state = {};
		rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_state.frontFace  = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state.depthClampEnable  = VK_FALSE;
		rasterization_state.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state.depthBiasEnable  = VK_FALSE;
		rasterization_state.depthBiasConstantFactor = 0.0f;
		rasterization_state.lineWidth = 1.0f;
		rasterization_state.depthBiasClamp = 0.0f;
		rasterization_state.depthBiasSlopeFactor = 0.0f;

		for (int32_t i = 0; i < 8; i++)
		{
			blend_attachment_states[i] = {};
			blend_attachment_states[i].colorWriteMask = (
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT
				);
			blend_attachment_states[i].blendEnable = VK_FALSE;
			blend_attachment_states[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blend_attachment_states[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blend_attachment_states[i].colorBlendOp = VK_BLEND_OP_ADD;
			blend_attachment_states[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blend_attachment_states[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blend_attachment_states[i].alphaBlendOp = VK_BLEND_OP_ADD;
		}

		depth_stencil_state = {};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = VK_TRUE;
		depth_stencil_state.depthWriteEnable = VK_TRUE;
		depth_stencil_state.depthCompareOp	= VK_COMPARE_OP_LESS_OR_EQUAL;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.back.compareOp 	= VK_COMPARE_OP_ALWAYS;
		depth_stencil_state.stencilTestEnable 	= VK_TRUE;
		depth_stencil_state.front = depth_stencil_state.back;

		multisample_state = {};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.pSampleMask = nullptr;

		tessellation_state = {};
		tessellation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellation_state.patchControlPoints = 0;
	}

	void vk_pipeline_info::fill_sahder_stages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStages)
	{
		if (vert_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
			shader_stage_create_info.module = vert_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}

		if (frag_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
			shader_stage_create_info.module = frag_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}

		if (comp_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
			shader_stage_create_info.module = comp_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}

		if (geom_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
			shader_stage_create_info.module = geom_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}

		if (tesc_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			shader_stage_create_info.module = tesc_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}

		if (tese_shader_module != VK_NULL_HANDLE) 
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info {};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			shader_stage_create_info.module = tese_shader_module;
			shader_stage_create_info.pName  = "main";
			shaderStages.push_back(shader_stage_create_info);
		}
	}

	std::shared_ptr<vk_pipeline> vk_pipeline::create_multi_binding(
		vk_device* in_device,
		VkPipelineCache pipelineCache,
		vk_pipeline_info& pipelineInfo,
		const std::vector<VkVertexInputBindingDescription>& input_bindings,
		const std::vector<VkVertexInputAttributeDescription>& vertex_input_attributs,
		VkPipelineLayout in_pipeline_layout,
		VkRenderPass renderPass,
		std::vector<VkDynamicState> dynamicStateEnables)
	{
		auto ret = std::make_shared<vk_pipeline>(in_device,in_pipeline_layout);
		
		VkPipelineVertexInputStateCreateInfo vertex_input_state{ };
		vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state.vertexBindingDescriptionCount = (uint32_t)input_bindings.size();
		vertex_input_state.pVertexBindingDescriptions = input_bindings.data();
		vertex_input_state.vertexAttributeDescriptionCount = (uint32_t)vertex_input_attributs.size();
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributs.data();

		VkPipelineColorBlendStateCreateInfo color_blend_state{ };
		color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state.attachmentCount = pipelineInfo.color_attachment_count;
		color_blend_state.pAttachments = pipelineInfo.blend_attachment_states;

		VkPipelineViewportStateCreateInfo viewportState{ };
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;

		VkPipelineDynamicStateCreateInfo dynamicState{ };
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStateEnables.size();
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		if (pipelineInfo.shaders) 
			shaderStages = pipelineInfo.shaders->shader_stage_create_infos;
		else 
			pipelineInfo.fill_sahder_stages(shaderStages);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{ };
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = in_pipeline_layout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.subpass = pipelineInfo.subpass;
		pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();

		pipelineCreateInfo.pVertexInputState = &vertex_input_state;
		pipelineCreateInfo.pInputAssemblyState = &(pipelineInfo.input_assembly_state);
		pipelineCreateInfo.pRasterizationState = &(pipelineInfo.rasterization_state);
		pipelineCreateInfo.pColorBlendState = &color_blend_state;
		pipelineCreateInfo.pMultisampleState = &(pipelineInfo.multisample_state);
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &(pipelineInfo.depth_stencil_state);
		pipelineCreateInfo.pDynamicState = &dynamicState;

		if (pipelineInfo.tessellation_state.patchControlPoints != 0) 
		{
			pipelineCreateInfo.pTessellationState = &(pipelineInfo.tessellation_state);
		}

		vk_check(vkCreateGraphicsPipelines(*in_device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &(ret->pipeline)));

		return ret;
	}

	std::shared_ptr<vk_pipeline> vk_pipeline::create_by_shader(
		vk_device* in_device,
		VkPipelineCache pipelineCache,
		vk_pipeline_info& pipelineInfo,
		std::shared_ptr<vk_shader_mix> shaders,
		VkRenderPass renderPass,
		std::vector<VkDynamicState> dynamicStateEnables)
	{
		return create_multi_binding(
			in_device,
			pipelineCache,
			pipelineInfo,
			shaders->input_bindings,
			shaders->input_attributes,
			shaders->pipeline_layout,
			renderPass,
			dynamicStateEnables
		);
	}

	std::shared_ptr<vk_pipeline> vk_pipeline::create_single_binding(vk_device* in_device,VkPipelineCache pipelineCache,vk_pipeline_info& pipelineInfo,VkVertexInputBindingDescription& inputBindings,const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes,VkPipelineLayout pipelineLayout,VkRenderPass renderPass,std::vector<VkDynamicState> dynamicStateEnables)
	{
		return create_multi_binding	(in_device,pipelineCache,pipelineInfo,{inputBindings},vertexInputAttributes,pipelineLayout,renderPass,dynamicStateEnables);
	}

	void vk_pipeline::bind(VkCommandBuffer cmd_buf,VkPipelineBindPoint bind_point)
	{
		vkCmdBindPipeline(cmd_buf, bind_point, pipeline);
	}
}}
