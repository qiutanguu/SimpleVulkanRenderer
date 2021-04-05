#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_vertex_buffer.h"
#include "global_uniform_buffers.h"

namespace flower{ namespace graphics{

	// 标准顶点 pos color uv0
	struct vertex_pcu
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv0;

		bool operator==(const vertex_pcu& other) const 
		{
			return pos == other.pos && color == other.color && uv0 == other.uv0;
		}

		static VkVertexInputBindingDescription get_binding() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(vertex_pcu);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 顶点偏移

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> get_attributes() 
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			attributeDescriptions.resize(3);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(vertex_pcu, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(vertex_pcu, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(vertex_pcu, uv0);

			return attributeDescriptions;
		}
	};

	
	// 按材质划分Mesh
	struct mesh
	{
		using using_vertex = vertex_pcu;

		std::vector<using_vertex> vertices;
		std::vector<uint32_t> indices;

		void load_obj_mesh(std::string path);

		void load_obj_mesh_new(std::string mesh_path,std::string mat_path);
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