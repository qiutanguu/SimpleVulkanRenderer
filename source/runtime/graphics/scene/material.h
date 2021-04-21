#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_vertex_buffer.h"
#include "../global_uniform_buffers.h"
#include "../vk/vk_pipeline.h"
#include "vertex.h"

namespace flower{namespace graphics{

	class material
	{
	public:
		uint32_t map_diffuse; // map_Kd
		uint32_t map_metalic; // map_Ks
		uint32_t map_mask; // map_d
		uint32_t map_normal; // map_bump
		uint32_t map_roughness; // map_Ns
		bool maps_set = false; // 是否设置了纹理

		material(){ }
		virtual ~material(){ }
		
		std::shared_ptr<vk_buffer> model_ubo; // 模型矩阵
		std::shared_ptr<vk_pipeline> pipeline; // 管线
		std::vector<std::shared_ptr<vk_descriptor_set>> descriptor_sets; // 描述符集


		void create_renderpipeline(std::shared_ptr<vk_shader_mix> in_shader,
			vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass);

		void recreate_swapchain(std::shared_ptr<vk_shader_mix> in_shader,
			vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass);
	};
}}
