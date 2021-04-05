#include "vk_pipeline.h"

namespace flower { namespace graphics{

	std::shared_ptr<vk_pipeline> vk_pipeline::create(
		vk_device* in_device,
		VkPipelineCache pipeline_cache,
		vk_pipeline_info& pipeline_info,
		const std::vector<VkVertexInputBindingDescription>& input_bindings,
		const std::vector<VkVertexInputAttributeDescription>& vertex_input_attributs,
		VkPipelineLayout* pipeline_layout,
		VkRenderPass* render_pass)
	{
		std::shared_ptr<vk_pipeline> ret = std::make_shared<vk_pipeline>(in_device);
		ret->pipeline_layout = *pipeline_layout;

		VkPipelineVertexInputStateCreateInfo vertexInputState {};
		
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = input_bindings.size();
		vertexInputState.pVertexBindingDescriptions = input_bindings.data();
		vertexInputState.vertexAttributeDescriptionCount = vertex_input_attributs.size();
		vertexInputState.pVertexAttributeDescriptions = vertex_input_attributs.data();

		VkPipelineColorBlendStateCreateInfo colorBlendState { };
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY; 
		colorBlendState.attachmentCount = pipeline_info.colorAttachmentCount;
		colorBlendState.pAttachments = pipeline_info.blendAttachmentStates;
		colorBlendState.blendConstants[0] = 0.0f; 
		colorBlendState.blendConstants[1] = 0.0f; 
		colorBlendState.blendConstants[2] = 0.0f; 
		colorBlendState.blendConstants[3] = 0.0f; 

		VkPipelineViewportStateCreateInfo viewportState { };
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState{ };
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		pipeline_info.fill_shader_stages(shaderStages);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo { };
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = *pipeline_layout;
		pipelineCreateInfo.renderPass = *render_pass;
		pipelineCreateInfo.subpass = pipeline_info.subpass;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &(pipeline_info.inputAssemblyState);
		pipelineCreateInfo.pRasterizationState = &(pipeline_info.rasterizationState);
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &(pipeline_info.multisampleState);
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &(pipeline_info.depthStencilState);
		pipelineCreateInfo.pDynamicState = &dynamicState;

		if(pipeline_info.tessellationState.patchControlPoints!=0)
		{
			pipelineCreateInfo.pTessellationState = &(pipeline_info.tessellationState);
		}
		vk_check(vkCreateGraphicsPipelines(*in_device,VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&(ret->pipeline)));

		return ret;
	}

} }
