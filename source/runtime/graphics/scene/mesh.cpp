#include "mesh.h"
#include "tinyobjloader/tiny_obj_loader.h"

namespace flower{ namespace graphics{

	void mesh::load_obj_mesh_new(vk_device* indevice,VkCommandPool inpool,std::string path,std::string mat_path)
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = mat_path.c_str();

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, reader_config)) {
			if (!reader.Error().empty()) 
			{
				LOG_IO_FATAL("obj�����ȡ����{0}",reader.Error());
			}
		}

		if (!reader.Warning().empty()) 
		{
			LOG_IO_WARN("obj�����ȡ���棺{0}",reader.Warning());
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		std::unordered_map<using_vertex, uint32_t> uniqueVertices{};
		sub_meshes.resize(materials.size());

		// ����ģ�;���
		VkDeviceSize bufferSize = sizeof(glm::mat4);
		for (auto& submesh : sub_meshes)
		{
			auto buffer = vk_buffer::create(
				*indevice,
				inpool,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				nullptr
			);

			glm::mat4 model_mat = glm::rotate(glm::mat4(1.0f),glm::radians(0.0f),glm::vec3(-1.0f,0.0f,0.0f));

			buffer->map();
			buffer->copy_to((void*)&model_mat,sizeof(model_mat));
			buffer->unmap();

			submesh.buffer_ubo_model = buffer;
		}

		// ����ÿ������
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			// ����ÿ����
			size_t index_offset = 0;


			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
			{
				auto sub_mesh_id = shapes[s].mesh.material_ids[f];
				if(sub_meshes.size()>0 && !sub_meshes[sub_mesh_id].material_using.map_Kd_set)
				{
					sub_meshes[sub_mesh_id].material_using.map_Kd = materials[sub_mesh_id].diffuse_texname;
					sub_meshes[sub_mesh_id].material_using.map_Kd_set = true;
				}
				
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// ����ÿ����Ķ���
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

					/*
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
					}
					*/

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

						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.x);
						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.y);
						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.z);

						vertices_data.push_back(one_vertex.pos.x);
						vertices_data.push_back(one_vertex.pos.y);
						vertices_data.push_back(one_vertex.pos.z);

						vertices_data.push_back(one_vertex.color.x);
						vertices_data.push_back(one_vertex.color.y);
						vertices_data.push_back(one_vertex.color.z);

						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.x);
						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.y);
						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.z);

						vertices_data.push_back(one_vertex.uv0.x);
						vertices_data.push_back(one_vertex.uv0.y);

						raw_data.get_stream(vertex_attribute::uv0).data.push_back(one_vertex.uv0.x);
						raw_data.get_stream(vertex_attribute::uv0).data.push_back(one_vertex.uv0.y);
					}

					indices.push_back(uniqueVertices[one_vertex]);

					if(sub_meshes.size()>0)
					{
						auto& subMesh_indices = sub_meshes[sub_mesh_id].indices;
						subMesh_indices.push_back(uniqueVertices[one_vertex]);
					}
				}
				index_offset += fv;
			}
		}
	}
} }