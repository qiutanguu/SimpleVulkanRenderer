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
#include "asset_system/asset_pmx.h"
#include "asset_system/asset_vmd.h"

namespace flower{ namespace graphics{

	class quad_mesh
	{
	public:
		quad_mesh(){ }
		~quad_mesh(){ }

		struct quad_vertex {
			float pos[3];
			float uv[2];
		};

		std::vector<float> vertices =
		{
			// pos[3]             uv[2]
			 1.0f,  1.0f, 0.0f ,  1.0f, 1.0f ,
			-1.0f,  1.0f, 0.0f ,  0.0f, 1.0f ,
			-1.0f, -1.0f, 0.0f ,  0.0f, 0.0f ,
			 1.0f, -1.0f, 0.0f ,  1.0f, 0.0f 
		};

		std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };

		std::shared_ptr<vk_vertex_buffer> vertex_buffer;
		std::shared_ptr<vk_index_buffer> index_buffer;

		void initialize(vk_device* in_device,VkCommandPool in_pool);
		void release() { vertex_buffer.reset();index_buffer.reset(); }
	};

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
			for (auto& val : has_registered)
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

		virtual ~mesh(){ }

		std::array<std::shared_ptr<vk_vertex_buffer>,renderpass_type::max_index> vertex_bufs = { };
		std::array<bool,renderpass_type::max_index> has_registered = { };

		std::vector<sub_mesh> sub_meshes;

		virtual void draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t pass_type);

		// �ڴ˴��洢�����ж���
		vertex_raw_data raw_data = {};

		// ע��render pass ��Ӧ�� mesh
		void register_renderpass(std::shared_ptr<vk_renderpass> pass,std::shared_ptr<vk_shader_mix> shader,uint32_t pass_type,bool reload_vertex_buf = true);


	protected:
		vk_device* device;
		VkCommandPool pool;
		
		void load_obj_mesh(
			std::string mesh_path,
			std::string mat_path,
			const glm::mat4& model = glm::mat4(1.0f)
		);

		friend class meshes_manager;
	};

	struct bone_node_mmd
	{
	public:
		bone_node_mmd(asset::PMXBone& in_bone) : bone(in_bone){ }

		asset::PMXBone& bone;
		int32_t index;

		bone_node_mmd* parent;
		std::vector<bone_node_mmd*> childs;

		// �Ӹ��ڵ㵽�ýڵ�����ձ任����
		glm::mat4 conv_matrix;
		
	};

	// �����㼶��
	struct bone_processor_mmd
	{
	public:
		bone_processor_mmd(asset::PMXFile* file);
		~bone_processor_mmd();
		bone_node_mmd* operator[](std::u16string name);

		// ����
		std::vector<bone_node_mmd*> list;

		// pmx �ļ������ж�����ڵ㡣
		std::vector<bone_node_mmd*> roots;
	};

	class mesh_mmd : public mesh
	{
	public:
		mesh_mmd(vk_device* indevice,
			VkCommandPool pool): mesh(indevice,pool)
		{

		}

		~mesh_mmd()
		{
			if(bone_manager!=nullptr)
			{
				delete bone_manager;
			}
		}

		void initialize(std::string mesh_path,std::string vmd_path);

		// �����㼶��
		bone_processor_mmd* bone_manager = nullptr;

		asset::VMDFile dance_data;
		asset::PMXFile miku_skeleton_mesh;

	private:	
		void load_pmx_mesh(
			std::string mesh_path
		);

		void load_vmd_data(
			std::string vmd_path
		);
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
			miku_mesh.reset();
		}

	public:
		std::shared_ptr<mesh> sponza_mesh;
		std::shared_ptr<mesh_mmd> miku_mesh;

		// NOTE: �����Global Identical Mat4
		static glm::mat4 get_mesh_transform(const glm::vec3& scale,const glm::vec3& pos, const glm::vec3& rotate);

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

