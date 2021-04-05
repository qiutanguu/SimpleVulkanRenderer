#pragma once
#include "vk_common.h"
#include "vk_device.h"
#include "vk_swapchain.h"

namespace flower{ namespace graphics{

	struct vk_viewport_scissor
	{
		VkViewport viewport{};
		VkRect2D scissor{};

		vk_viewport_scissor(vk_swapchain* in_swapchain,float width = -1.0f,float height = -1.0f,float x = 0.0f,float y = 0.0f)
		{
			viewport.x = x;
			viewport.y = y;

			if(width<0)
			{
				viewport.width = in_swapchain->get_swapchain_extent().width;
			}
			else
			{
				viewport.width = width;
			}

			if(height<0)
			{
				viewport.height = in_swapchain->get_swapchain_extent().height;
			}
			else
			{
				viewport.height = height;
			}

			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			scissor.offset = {0, 0};
			scissor.extent = in_swapchain->get_swapchain_extent();
		}
		vk_viewport_scissor() = default;
		~vk_viewport_scissor() = default;
	};
	
	struct vk_pipeline_info
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState { };
		VkPipelineRasterizationStateCreateInfo rasterizationState { };
		VkPipelineColorBlendAttachmentState	blendAttachmentStates[8] { };
		VkPipelineDepthStencilStateCreateInfo depthStencilState { };
		VkPipelineMultisampleStateCreateInfo multisampleState { };
		VkPipelineTessellationStateCreateInfo tessellationState { };

		VkShaderModule vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule fragShaderModule = VK_NULL_HANDLE;
		VkShaderModule compShaderModule = VK_NULL_HANDLE;
		VkShaderModule tescShaderModule = VK_NULL_HANDLE;
		VkShaderModule teseShaderModule = VK_NULL_HANDLE;
		VkShaderModule geomShaderModule = VK_NULL_HANDLE;

		int32_t	subpass = 0;
		int32_t colorAttachmentCount = 1;

		vk_pipeline_info()
		{
			// 1. 输入装配
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyState.primitiveRestartEnable = VK_FALSE;

			// 3. 光栅化器配置
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.lineWidth = 1.0f;
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.depthBiasConstantFactor = 0.0f; 
			rasterizationState.depthBiasClamp = 0.0f; 
			rasterizationState.depthBiasSlopeFactor = 0.0f; 

			// 4. MSAA配置
			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.sampleShadingEnable = VK_FALSE;
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleState.minSampleShading = 1.0f;
			multisampleState.pSampleMask = nullptr;
			multisampleState.alphaToCoverageEnable = VK_FALSE; 
			multisampleState.alphaToOneEnable = VK_FALSE;


			// 5. 混合配置
			for(int32_t i = 0; i<8; ++i)
			{
				blendAttachmentStates[i] = {};
				blendAttachmentStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
				blendAttachmentStates[i].blendEnable = VK_FALSE;
				blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
				blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
				blendAttachmentStates[i].blendEnable = VK_TRUE;
				blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				blendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
				blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
			}

			// 6. 深度模板
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.depthTestEnable = VK_TRUE;
			depthStencilState.depthWriteEnable = VK_TRUE;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState.stencilTestEnable = VK_TRUE;
			depthStencilState.minDepthBounds = 0.0f;
			depthStencilState.maxDepthBounds = 1.0f;
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.front = {};
			depthStencilState.back = {};
			
			tessellationState.sType =VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tessellationState.patchControlPoints = 0;
		}

		void fill_shader_stages(std::vector<VkPipelineShaderStageCreateInfo>& shader_stages)
		{
			if(vertShaderModule != VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				shaderStageCreateInfo.module = vertShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::VS;
				shader_stages.push_back(shaderStageCreateInfo);
			}

			if(fragShaderModule != VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				shaderStageCreateInfo.module = fragShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::FS;
				shader_stages.push_back(shaderStageCreateInfo);
			}

			if(compShaderModule != VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
				shaderStageCreateInfo.module = compShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::CS;
				shader_stages.push_back(shaderStageCreateInfo);
			}

			if(geomShaderModule != VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				shaderStageCreateInfo.module = geomShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::GS;
				shader_stages.push_back(shaderStageCreateInfo);
			}

			if(tescShaderModule!=VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				shaderStageCreateInfo.module = tescShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::TSC;
				shader_stages.push_back(shaderStageCreateInfo);
			}

			if(teseShaderModule!=VK_NULL_HANDLE)
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				shaderStageCreateInfo.module = teseShaderModule;
				shaderStageCreateInfo.pName = shader_main_function_name::TSE;
				shader_stages.push_back(shaderStageCreateInfo);
			}
		}
	};

	class vk_pipeline
	{
	public:

		vk_pipeline(vk_device* in_device)
			: device (in_device)
			, pipeline(VK_NULL_HANDLE)
		{

		}

		~vk_pipeline()
		{
			if(pipeline!=VK_NULL_HANDLE)
			{
				vkDestroyPipeline(*device,pipeline,nullptr);
			}
		}

		static std::shared_ptr<vk_pipeline> create(
			vk_device* in_device,
			VkPipelineCache pipeline_cache,
			vk_pipeline_info& pipeline_info,
			const std::vector<VkVertexInputBindingDescription>& input_bindings,
			const std::vector<VkVertexInputAttributeDescription>& vertex_input_attributs,
			VkPipelineLayout* pipeline_layout,
			VkRenderPass* render_pass
		);
	private:
		vk_device* device;

	public:
		VkPipeline			pipeline;
		VkPipelineLayout	pipeline_layout;
	};

}}