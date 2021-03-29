#include "mesh.h"
#include "tinyobjloader/tiny_obj_loader.h"

namespace flower{ namespace graphics{

	void mesh::load_obj_mesh(std::string path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) 
		{
			LOG_IO_FATAL("º”‘ÿƒ£–Õ{0} ß∞‹:{1}",path.c_str(),(warn + err).c_str());
		}

		std::unordered_map<vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) 
		{
			for (const auto& index : shape.mesh.indices) 
			{
				vertex one_vertex{};

				one_vertex.pos = 
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				one_vertex.texCoord = 
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				one_vertex.color = {1.0f, 1.0f, 1.0f};

				if (uniqueVertices.count(one_vertex) == 0) 
				{
					uniqueVertices[one_vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(one_vertex);
				}

				indices.push_back(uniqueVertices[one_vertex]);
			}
		}
	}

} }