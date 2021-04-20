#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_vertex_buffer.h"
#include "../global_uniform_buffers.h"
#include "vertex.h"

namespace flower{ namespace graphics{

	struct vertex_interface
	{
		vertex_interface() { }
		virtual ~vertex_interface(){ }
	};

	// 标准顶点
	struct vertex_standard : public vertex_interface
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv0;
		glm::vec3 normal;
		glm::vec4 tangent;

		bool operator==(const vertex_standard& other) const 
		{
			return pos == other.pos && 
				   color == other.color && 
				   uv0 == other.uv0 &&
				   normal == other.normal &&
				   tangent == other.tangent;
		}
	};

	struct material
	{
		std::string map_Kd; // diffuse

	};

	struct sub_mesh
	{
		std::vector<uint32_t> indices;

		// 每个submesh都有一个model矩阵
		glm::mat4 model;

		std::shared_ptr<vk_buffer> buffer_ubo_model;

		material material_using;
	};
	
	// 按材质划分Mesh
	struct mesh
	{
		std::vector<sub_mesh> sub_meshes;
		std::vector<float> vertices_data;

		vertex_raw_data raw_data = {};

		// 存储的所有顶点
		std::vector<using_vertex> vertices;
		std::vector<uint32_t> indices;

		void load_obj_mesh_new(vk_device* indevice,VkCommandPool inpool,std::string mesh_path,std::string mat_path);
	};

} }

namespace std 
{
	template<> struct hash<flower::graphics::vertex_standard> 
	{
		size_t operator()(flower::graphics::vertex_standard const& vertex) const 
		{
			return 
				((hash<glm::vec3>()(vertex.pos) ^
				 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				 (hash<glm::vec2>()(vertex.uv0) << 1);
		}
	};
}