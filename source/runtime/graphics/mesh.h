#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_vertex_buffer.h"
#include "global_uniform_buffers.h"

#include "scene/vertex.h"

namespace flower{ namespace graphics{

	// ��׼���� pos color uv0
	struct vertex_pcu
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv0;

		bool operator==(const vertex_pcu& other) const 
		{
			return pos == other.pos && color == other.color && uv0 == other.uv0;
		}
	};

	struct material
	{
		std::string map_Kd; // diffuse
		bool map_Kd_set  = false;

	};

	struct sub_mesh
	{
		std::vector<uint32_t> indices;

		// ÿ��submesh����һ��model����
		glm::mat4 model;

		std::shared_ptr<vk_buffer> buffer_ubo_model;

		material material_using;
	};
	
	// �����ʻ���Mesh
	struct mesh
	{
		using using_vertex = vertex_pcu;

		std::vector<sub_mesh> sub_meshes;

		std::vector<float> vertices_data;

		vertex_raw_data raw_data = {};

		std::vector<vertex_attribute>  vertices_attributes = {
			vertex_attribute::pos, 
			vertex_attribute::color, 
			vertex_attribute::uv0  
		};

		// �洢�����ж���
		std::vector<using_vertex> vertices;
		std::vector<uint32_t> indices;

		void load_obj_mesh_new(vk_device* indevice,VkCommandPool inpool,std::string mesh_path,std::string mat_path);

		void load_gltf_mesh();
	};

} }

namespace std 
{
	template<> struct hash<flower::graphics::vertex_pcu> 
	{
		size_t operator()(flower::graphics::vertex_pcu const& vertex) const 
		{
			return 
				((hash<glm::vec3>()(vertex.pos) ^
				 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				 (hash<glm::vec2>()(vertex.uv0) << 1);
		}
	};
}