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
			LOG_IO_FATAL("加载模型{0}失败:{1}",path.c_str(),(warn + err).c_str());
		}

		std::unordered_map<using_vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) 
		{
			for (const auto& index : shape.mesh.indices) 
			{
				using_vertex one_vertex{};

				one_vertex.pos = 
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				one_vertex.uv0 = 
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

	void mesh::load_obj_mesh_new(std::string path,std::string mat_path)
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = mat_path.c_str();

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, reader_config)) {
			if (!reader.Error().empty()) 
			{
				LOG_IO_FATAL("obj网格读取错误：{0}",reader.Error());
			}
		}

		if (!reader.Warning().empty()) 
		{
			LOG_IO_WARN("obj网格读取警告：{0}",reader.Warning());
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		std::unordered_map<using_vertex, uint32_t> uniqueVertices{};

		// 遍历每个网格
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			// 遍历每个面
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
			{
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// 遍历每个面的顶点
				for (size_t v = 0; v < fv; v++) 
				{
					using_vertex one_vertex{};

					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

					one_vertex.pos = {
						vx,
						vy,
						vz
					};

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) 
					{
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) 
					{
						tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
						tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

						one_vertex.uv0 = {
							tx,
							1.0f - ty
						};
					}

					one_vertex.color = {1.0f,1.0f,1.0f};

					if (uniqueVertices.count(one_vertex) == 0) 
					{
						uniqueVertices[one_vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(one_vertex);
					}

					indices.push_back(uniqueVertices[one_vertex]);
				}
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}
	}
} }