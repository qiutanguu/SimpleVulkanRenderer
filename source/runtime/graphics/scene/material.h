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

		std::shared_ptr<vk_buffer> model_ubo; // ģ�;���
		std::shared_ptr<vk_shader_mix> shader; // ʹ�õ�shader
		std::shared_ptr<vk_pipeline> pipeline; // ����
		std::shared_ptr<vk_descriptor_set> descriptor_set; // ��������
	};
}}
