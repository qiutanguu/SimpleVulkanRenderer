#pragma once
#include "vk_common.h"
#include "vk_device.h"
#include "vk_swapchain.h"

namespace flower{ namespace graphics{

	struct pipeline
	{
		static void input_assembly_config(VkPipelineInputAssemblyStateCreateInfo& inout);

		static void viewport_config(VkPipelineViewportStateCreateInfo& inout,VkViewport* viewport,VkRect2D* scissor);

		static void msaa_config(VkPipelineMultisampleStateCreateInfo& inout);

		static void rasterizer_config(VkPipelineRasterizationStateCreateInfo& inout);

		static void colorBlendAttachment_config(VkPipelineColorBlendAttachmentState& inout);

		static void colorBlend_config(VkPipelineColorBlendStateCreateInfo& inout,VkPipelineColorBlendAttachmentState* attachment);

		static void depthStencil_config(VkPipelineDepthStencilStateCreateInfo& inout);

		static void shader_vertex_config(VkPipelineShaderStageCreateInfo& inout,VkShaderModule shaderModule);

		static void shader_fragment_config(VkPipelineShaderStageCreateInfo& inout,VkShaderModule shaderModule);
	};

}}