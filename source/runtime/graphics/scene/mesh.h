#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_vertex_buffer.h"
#include "../global_uniform_buffers.h"
#include "../vk/vk_pipeline.h"
#include "vertex.h"
#include "material.h"

namespace flower{ namespace graphics{

	class sub_mesh
	{
	public:
		sub_mesh(){ }
		~sub_mesh(){ }

		// sub_mesh�Ĵ���
		std::vector<uint32_t> indices;

		// ÿ��submesh����һ��model����
		glm::mat4 model;

		// ÿ��submesh�Ĵ���buffer
		std::shared_ptr<vk_index_buffer> index_buf;
		
		// ʹ���еĲ���
		std::shared_ptr<material> mat;

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);
	};
	
	// �����ʻ���Mesh
	class mesh
	{
	public:
		mesh(vk_device* indevice,
			VkCommandPool pool,
			std::shared_ptr<vk_shader_mix> shader): 
			device(indevice),
			pool(pool),
			shader(shader)
		{
			
		}

		~mesh(){ }

		std::shared_ptr<vk_vertex_buffer> vertex_buf;
		std::vector<sub_mesh> sub_meshes;

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);

		// �ڴ˴��洢�����ж���
		vertex_raw_data raw_data = {};

		void load_obj_mesh(
			vk_device* indevice,
			VkCommandPool inpool,
			std::string mesh_path,
			std::string mat_path,
			VkRenderPass renderpass,
			vk_swapchain* inswapchain,
			material_type mat_type
		);

		void on_swapchain_recreate(vk_swapchain* inswapchain,VkRenderPass renderpass)
		{
			for(auto& submesh : sub_meshes)
			{
				submesh.mat->on_swapchain_recreate(device,inswapchain,renderpass);
			}
		}

	private:
		vk_device* device;
		std::shared_ptr<vk_shader_mix> shader;
		VkCommandPool pool;

		void upload_buffer();
	};

} }

