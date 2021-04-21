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

		// sub_mesh的次序
		std::vector<uint32_t> indices;

		// 每个submesh都有一个model矩阵
		glm::mat4 model;

		// 每个submesh的次序buffer
		std::shared_ptr<vk_index_buffer> index_buf;
		
		// 使用中的材质
		std::shared_ptr<material> mat;

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);
	};
	
	// 按材质划分Mesh
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

		// 在此处存储的所有顶点
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

