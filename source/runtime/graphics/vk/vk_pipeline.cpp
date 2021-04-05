#include "vk_pipeline.h"

namespace flower { namespace graphics{

	void pipeline::input_assembly_config(VkPipelineInputAssemblyStateCreateInfo& inputAssembly)
	{
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void pipeline::viewport_config(VkPipelineViewportStateCreateInfo& viewportState,VkViewport* viewport,VkRect2D* scissor)
	{
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = scissor;
	}

	void pipeline::msaa_config(VkPipelineMultisampleStateCreateInfo& inout)
	{
		inout.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		inout.sampleShadingEnable = VK_FALSE;
		inout.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		inout.minSampleShading = 1.0f;
		inout.pSampleMask = nullptr;
		inout.alphaToCoverageEnable = VK_FALSE;
		inout.alphaToOneEnable = VK_FALSE;
	}

	void pipeline::rasterizer_config(VkPipelineRasterizationStateCreateInfo& inout)
	{
		inout.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		inout.depthClampEnable = VK_FALSE;
		inout.rasterizerDiscardEnable = VK_FALSE;
		inout.polygonMode = VK_POLYGON_MODE_FILL;
		inout.lineWidth = 1.0f;
		inout.cullMode = VK_CULL_MODE_NONE;
		inout.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		inout.depthBiasEnable = VK_FALSE;
		inout.depthBiasConstantFactor = 0.0f;
		inout.depthBiasClamp = 0.0f;
		inout.depthBiasSlopeFactor = 0.0f;
	}

	void pipeline::colorBlendAttachment_config(VkPipelineColorBlendAttachmentState& inout)
	{
		inout.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
		inout.blendEnable = VK_FALSE;
		inout.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		inout.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		inout.colorBlendOp = VK_BLEND_OP_ADD;
		inout.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		inout.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		inout.alphaBlendOp = VK_BLEND_OP_ADD;
		inout.blendEnable = VK_TRUE;
		inout.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		inout.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		inout.colorBlendOp = VK_BLEND_OP_ADD;
		inout.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		inout.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		inout.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void pipeline::colorBlend_config(VkPipelineColorBlendStateCreateInfo& inout,
	VkPipelineColorBlendAttachmentState* attachment)
	{
		inout.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		inout.logicOpEnable = VK_FALSE;
		inout.logicOp = VK_LOGIC_OP_COPY;
		inout.attachmentCount = 1;
		inout.pAttachments = attachment;
		inout.blendConstants[0] = 0.0f;
		inout.blendConstants[1] = 0.0f;
		inout.blendConstants[2] = 0.0f;
		inout.blendConstants[3] = 0.0f;
	}

	void pipeline::depthStencil_config(VkPipelineDepthStencilStateCreateInfo& depthStencil)
	{
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
	}

	void pipeline::shader_vertex_config(VkPipelineShaderStageCreateInfo& vertShaderStageInfo,VkShaderModule shaderModule)
	{
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shaderModule;
		vertShaderStageInfo.pName = shader_main_function_name::VS;
	}

	void pipeline::shader_fragment_config(VkPipelineShaderStageCreateInfo& fragShaderStageInfo,VkShaderModule shaderModule)
	{
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shaderModule;
		fragShaderStageInfo.pName = shader_main_function_name::FS;
	}

	

}}
