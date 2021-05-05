#pragma once
#include "vk_common.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_shader.h"

namespace flower{ namespace graphics{

	inline VkPipelineColorBlendAttachmentState vk_pipeline_color_blend_attachment_state
	(
		VkColorComponentFlags colorWriteMask,
		VkBool32 blendEnable
	)
	{
		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
		pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
		pipelineColorBlendAttachmentState.blendEnable = blendEnable;
		return pipelineColorBlendAttachmentState;
	}

	inline VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info(
		uint32_t attachmentCount,
		const VkPipelineColorBlendAttachmentState* pAttachments)
	{
		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
		pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
		pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
		return pipelineColorBlendStateCreateInfo;
	}


	struct vk_pipeline_info
	{
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineColorBlendAttachmentState blend_attachment_states[8];
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineTessellationStateCreateInfo tessellation_state;

		VkShaderModule	vert_shader_module = VK_NULL_HANDLE;
		VkShaderModule	frag_shader_module = VK_NULL_HANDLE;
		VkShaderModule	comp_shader_module = VK_NULL_HANDLE;
		VkShaderModule	tesc_shader_module = VK_NULL_HANDLE;
		VkShaderModule	tese_shader_module = VK_NULL_HANDLE;
		VkShaderModule	geom_shader_module = VK_NULL_HANDLE;

		std::shared_ptr<vk_shader_mix> shaders = nullptr;

		int32_t	subpass = 0;
		int32_t color_attachment_count = 1;

		vk_pipeline_info();
		void fill_sahder_stages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStages);
	};

	class vk_pipeline
	{
	public:
		vk_pipeline(vk_device* in_device,VkPipelineLayout in_pipeline_layout) : device(in_device),layout(in_pipeline_layout) {  }

		~vk_pipeline()
		{
			if (pipeline != VK_NULL_HANDLE) 
			{
				vkDestroyPipeline(*device, pipeline, nullptr);
			}
		}

		void bind(VkCommandBuffer cmd_buf,VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);

		static std::shared_ptr<vk_pipeline> create_multi_binding(
			vk_device* in_device,
			VkPipelineCache pipelineCache,
			vk_pipeline_info& pipelineInfo, 
			const std::vector<VkVertexInputBindingDescription>& inputBindings, 
			const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes,
			VkPipelineLayout pipelineLayout,
			VkRenderPass renderPass,
			std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}
		);

		static std::shared_ptr<vk_pipeline> create_single_binding(
			vk_device* in_device,
			VkPipelineCache pipelineCache,
			vk_pipeline_info& pipelineInfo,
			VkVertexInputBindingDescription& inputBindings,
			const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes,
			VkPipelineLayout pipelineLayout,
			VkRenderPass renderPass,
			std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}
		);

		static std::shared_ptr<vk_pipeline> create_by_shader(
			vk_device* in_device,
			VkPipelineCache pipelineCache,
			vk_pipeline_info& pipelineInfo,
			std::shared_ptr<vk_shader_mix> shaders,
			VkRenderPass renderPass,
			std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}
		);
		
	private:
		vk_device* device;
	public:
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

}}