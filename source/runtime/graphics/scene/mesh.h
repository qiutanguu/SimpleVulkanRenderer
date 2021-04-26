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

	namespace texture_id_type
	{
		constexpr auto diffuse = 0;
		constexpr auto mask = 1;
		constexpr auto metallic = 2;
		constexpr auto normal = 3;
		constexpr auto roughness = 4;
	}

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
		
		// ����ids
		std::vector<uint32_t> texture_ids = {};

		// ÿ��renderpass��Ӧ��ע���Ӧ��material
		std::array<std::shared_ptr<material>,renderpass_type::max_index> mat_map = { }; 
		std::array<bool,renderpass_type::max_index> has_registered = { };

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t passtype);

		void register_renderpass(int32_t passtype,vk_device* indevice,
			VkRenderPass in_renderpass,
			VkCommandPool in_pool);
	};
	
	// �����ʻ���Mesh
	class mesh
	{
	public:
		mesh(vk_device* indevice,
			VkCommandPool pool): 
			device(indevice),
			pool(pool)
		{
			
		}

		~mesh(){ }

		std::array<std::shared_ptr<vk_vertex_buffer>,renderpass_type::max_index> vertex_bufs = { };
		std::array<bool,renderpass_type::max_index> has_registered = { };

		std::vector<sub_mesh> sub_meshes;

		virtual void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t pass_type);

		// �ڴ˴��洢�����ж���
		vertex_raw_data raw_data = {};

		// ע��render pass ��Ӧ�� mesh
		void register_renderpass(std::shared_ptr<vk_renderpass> pass,std::shared_ptr<vk_shader_mix> shader,bool reload_vertex_buf = true);


	protected:
		vk_device* device;
		VkCommandPool pool;
		
		void load_obj_mesh(
			std::string mesh_path,
			std::string mat_path
		);

		friend class meshes_manager;
	};


	class meshes_manager
	{
	public:
		meshes_manager() { };
		~meshes_manager() { };

		void initialize(vk_device* indevice,VkCommandPool inpool);

		// �ͷż��ص��ڴ��е���������
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

	inline void virtual_full_screen_triangle_draw(
		std::shared_ptr<vk_command_buffer> cmd_buf,
		std::shared_ptr<material> full_screen_mat)
	{
		full_screen_mat->pipeline->bind(*cmd_buf);

		vkCmdBindDescriptorSets(
			*cmd_buf,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			full_screen_mat->pipeline->layout,
			0,
			1,
			full_screen_mat->descriptor_set->descriptor_sets.data(),
			0,
			nullptr
		);

		vkCmdDraw(*cmd_buf, 3, 1, 0, 0);
	}

} }

