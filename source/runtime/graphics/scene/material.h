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

		std::shared_ptr<vk_buffer> model_ubo; // 模型矩阵
		std::shared_ptr<vk_shader_mix> shader; // 使用的shader
		std::shared_ptr<vk_pipeline> pipeline; // 管线
		std::shared_ptr<vk_descriptor_set> descriptor_set; // 描述符集
	};
}}
