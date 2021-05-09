#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/scene/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "graphics/vk/vk_texture.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/vk/vk_renderpass.h"
#include "graphics/vk/vk_device.h"
#include "graphics/vk/vk_command_buffer.h"
#include "graphics/scene_textures.h"

namespace flower{ namespace graphics{

	// ±ßÔµ¼ì²âµÄ compute shader
	class compute_edge_detect
	{
	public:
		compute_edge_detect(vk_device* in_device,VkCommandPool in_pool) : device(in_device),pool(in_pool){ }
		~compute_edge_detect();

		static std::shared_ptr<compute_edge_detect> create(vk_device* in_device,VkCommandPool in_pool);


		VkPipeline pipeline;
		VkSemaphore semaphore;     
		std::shared_ptr<vk_command_buffer> cmd_buf;
		std::shared_ptr<vk_texture> edge_detect_compute_target;

		std::shared_ptr<vk_descriptor_set> descriptor_set;

		void build_command_buffer();

	private:
		vk_device* device;
		VkCommandPool pool;

		std::vector<VkDescriptorSetLayout> descriptor_set_layout_ref;
		VkPipelineLayout pipeline_layout_ref;
 	};

} }