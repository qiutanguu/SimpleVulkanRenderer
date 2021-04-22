#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_vertex_buffer.h"
#include "../global_uniform_buffers.h"
#include "../vk/vk_pipeline.h"
#include "vertex.h"
#include "material.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "../vk/vk_renderpass.h"

namespace flower{ namespace graphics{

	class sub_mesh
	{
	public:
		sub_mesh()
		{
			for (auto& val:has_registered)
			{
				val = false;
			}
		}
		~sub_mesh(){ }

		// sub_mesh的次序
		std::vector<uint32_t> indices;

		// 每个submesh都有一个model矩阵
		glm::mat4 model;

		// 每个submesh的次序buffer
		std::shared_ptr<vk_index_buffer> index_buf;
		
		// 使用中的obj材质
		tinyobj::material_t mat_obj;

		// 每种renderpass都应该注册对应的material
		std::array<std::shared_ptr<material>,renderpass_type::max_index> size = { }; 
		std::array<bool,renderpass_type::max_index> has_registered = { };

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);

		// should rebuild when swapchain change.
		void register_renderpass(int32_t passtype);
	};
	
	// 按材质划分Mesh
	class mesh
	{
	public:
		mesh(vk_device* indevice,
			VkCommandPool pool,
			std::shared_ptr<vk_shader_mix> shader): 
			device(indevice),
			pool(pool)
		{
			
		}

		~mesh(){ }

		std::shared_ptr<vk_vertex_buffer> vertex_buf;
		std::vector<sub_mesh> sub_meshes;

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);

		// 在此处存储的所有顶点
		vertex_raw_data raw_data = {};

		void load_obj_mesh(
			std::string mesh_path,
			std::string mat_path
		);

	private:
		vk_device* device;
		VkCommandPool pool;
		void upload_buffer();
	};


	class meshes_manager
	{
	public:
		meshes_manager() { };
		~meshes_manager() { };

		void initialize(vk_device* indevice,VkCommandPool inpool);

		// 释放加载到内存中的
		void release_cpu_mesh_data();
		void release()
		{
			sponza_mesh.reset();
		}

	public:
		std::shared_ptr<mesh> sponza_mesh;
		

	private:
		vk_device* device;
		VkCommandPool pool;
	};

	extern meshes_manager g_meshes_manager;
} }

