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
		material(){ }
		virtual ~material(){ }

		virtual void on_swapchain_recreate(vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass) = 0;

		virtual void on_create(vk_device* indevice,VkRenderPass in_renderpass) = 0;

		std::shared_ptr<vk_buffer> model_ubo; // 模型矩阵
		std::shared_ptr<vk_shader_mix> shader; // 使用的shader
		std::shared_ptr<vk_pipeline> pipeline; // 管线
		std::shared_ptr<vk_descriptor_set> descriptor_set; // 描述符集
	};

	enum class material_type
	{
		gbuffer,
		texture_map,
		pbr_textures
	};

	class material_gbuffer : public material
	{
	public:
		uint32_t map_diffuse; // map_Kd
		uint32_t map_metalic; // map_Ks
		uint32_t map_mask; // map_d
		uint32_t map_normal; // map_bump
		uint32_t map_roughness; // map_Ns

		bool maps_set = false; // 是否设置了纹理

		material_gbuffer(){ }
		~material_gbuffer(){ }
		
		virtual void on_create(vk_device* indevice,VkRenderPass in_renderpass) override;
		virtual void on_swapchain_recreate(vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass) override;
	};
}}
