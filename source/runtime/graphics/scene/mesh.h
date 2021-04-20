#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_vertex_buffer.h"
#include "../global_uniform_buffers.h"
#include "../vk/vk_pipeline.h"
#include "vertex.h"

namespace flower{ namespace graphics{

	// 标准顶点
	struct vertex_standard
	{
		glm::vec3 pos = glm::vec3(0.0f);
		glm::vec3 color = glm::vec3(0.0f);
		glm::vec2 uv0 = glm::vec2(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);
		glm::vec4 tangent = glm::vec4(0.0f);

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
		uint32_t map_diffuse; // map_Kd
		uint32_t map_metalic; // map_Ks
		uint32_t map_mask; // map_d
		uint32_t map_normal; // map_bump
		uint32_t map_roughness; // map_Ns

		bool maps_set = false;
	};

	struct sub_mesh
	{
		// sub_mesh的次序
		std::vector<uint32_t> indices;

		// 每个submesh都有一个model矩阵
		glm::mat4 model;

		std::shared_ptr<vk_index_buffer> index_buf;
		std::shared_ptr<vk_buffer> buffer_ubo_model;

		std::shared_ptr<vk_descriptor_set> descriptor_set;

		material material_using;

		void bind();
		void draw(std::shared_ptr<vk_command_buffer> cmd_buf);
	};
	
	// 按材质划分Mesh
	struct mesh
	{
		std::shared_ptr<vk_pipeline> pipeline_render;
		std::shared_ptr<vk_shader_mix> shaders_render;

		std::shared_ptr<vk_vertex_buffer> vertex_buf;
		
		std::vector<sub_mesh> sub_meshes;

		void draw(std::shared_ptr<vk_command_buffer> cmd_buf);

		// 在此处存储的所有顶点
		vertex_raw_data raw_data = {};

		void load_obj_mesh(vk_device* indevice,VkCommandPool inpool,std::string mesh_path,std::string mat_path);
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
				 (hash<glm::vec2>()(vertex.uv0) << 1) ^
				 (hash<glm::vec3>()(vertex.normal) << 1) ^
				 (hash<glm::vec4>()(vertex.tangent) >> 1);
		}
	};
}