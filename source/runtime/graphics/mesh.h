#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"

namespace flower{ namespace graphics{

	struct uniform_buffer_mvp
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	// 标准顶点 pos color textureCoord
	struct vertex 
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		bool operator==(const vertex& other) const 
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}

		static VkVertexInputBindingDescription getBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 顶点偏移

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(vertex, texCoord);

			return attributeDescriptions;
		}
	};

	
	// 按材质划分Mesh
	struct mesh
	{
		std::vector<vertex> vertices;
		std::vector<uint32_t> indices;

		void load_obj_mesh(std::string path);
	};

} }

namespace std 
{
	template<> struct hash<flower::graphics::vertex> 
	{
		size_t operator()(flower::graphics::vertex const& vertex) const 
		{
			return 
				((hash<glm::vec3>()(vertex.pos) ^
				 (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				 (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}