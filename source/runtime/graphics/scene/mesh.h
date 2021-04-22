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

		// sub_mesh�Ĵ���
		std::vector<uint32_t> indices;

		// ÿ��submesh����һ��model����
		glm::mat4 model;

		// ÿ��submesh�Ĵ���buffer
		std::shared_ptr<vk_index_buffer> index_buf;
		
		// ʹ���е�obj����
		tinyobj::material_t mat_obj;

		// ÿ��renderpass��Ӧ��ע���Ӧ��material
		std::array<std::shared_ptr<material>,renderpass_type::max_index> size = { }; 
		std::array<bool,renderpass_type::max_index> has_registered = { };

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t index);

		// should rebuild when swapchain change.
		void register_renderpass(int32_t passtype);
	};
	
	// �����ʻ���Mesh
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

		// �ڴ˴��洢�����ж���
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

		// �ͷż��ص��ڴ��е�
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

