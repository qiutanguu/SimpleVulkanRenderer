#include "mesh.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "../texture_manager.h"

namespace flower{ namespace graphics{

	void sub_mesh::bind()
	{

		/*texture_descriptor_sets[index] = g_shader_manager.texture_map_shader->allocate_descriptor_set();
		descriptor_set->set_buffer("ub_vp",buffer_ubo_vp[i]);
		descriptor_set->set_buffer("ub_m",mesh_sponza.sub_meshes[j].buffer_ubo_model);

		descriptor_set->set_image("base_color_texture",g_texture_manager.get_texture_vk(mesh_sponza.sub_meshes[j].material_using.map_diffuse));*/
	}

	void sub_mesh::draw(std::shared_ptr<vk_command_buffer> cmd_buf)
	{
		/*vkCmdBindDescriptorSets(
			*cmd_buf,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_render->layout,
			0,
			1,
			descriptor_set->descriptor_sets.data(),
			0,
			nullptr
		);*/

		index_buf->bind_and_draw(*cmd_buf);
	}

	void mesh::draw(std::shared_ptr<vk_command_buffer> cmd_buf)
	{
		pipeline_render->bind(*cmd_buf);
		vertex_buf->bind(*cmd_buf);

		for (auto& submesh : sub_meshes)
		{
			submesh.draw(cmd_buf);
		}
	}

	void mesh::load_obj_mesh(vk_device* indevice,VkCommandPool inpool,std::string path,std::string mat_path)
	{
		std::string mtl_search_path;
		size_t pos = path.find_last_of("/\\");
		if(pos!=std::string::npos)
		{
			mtl_search_path = path.substr(0,pos);
		}

		mtl_search_path += "/";

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

		std::unordered_map<vertex_standard, uint32_t> uniqueVertices{};
		std::vector<vertex_standard> vertices;

		sub_meshes.resize(materials.size());

		// 设置模型矩阵
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

		// 遍历每个网格
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			// 遍历每个面
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
			{
				auto sub_mesh_id = shapes[s].mesh.material_ids[f];
				if(sub_meshes.size() > 0 && !sub_meshes[sub_mesh_id].material_using.maps_set)
				{
					auto& set_mat = sub_meshes[sub_mesh_id].material_using;
					auto& cat_mat = materials[sub_mesh_id];

					// 加载每种材质需要的纹理
					if(cat_mat.diffuse_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.diffuse_texname;
						set_mat.map_diffuse = g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_SRGB,
							sampler_layout::linear_repeat(),
							texpath
						);
					}
					else
					{
						set_mat.map_diffuse = g_texture_manager.checkboard;
					}

					if(cat_mat.alpha_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.alpha_texname;
						set_mat.map_mask = g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						);
					}
					else
					{
						set_mat.map_mask = g_texture_manager.white_16x16;
					}

					if(cat_mat.specular_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.specular_texname;
						set_mat.map_metalic = g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						);
					}
					else
					{
						set_mat.map_metalic = g_texture_manager.black_16x16;
					}

					if(cat_mat.bump_texname!="")
					{
						auto texpath = mtl_search_path+cat_mat.bump_texname;
						set_mat.map_normal = g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						);
					}
					else
					{
						set_mat.map_normal = g_texture_manager.default_normal;
					}

					if(cat_mat.specular_highlight_texname!="")
					{
						auto texpath = mtl_search_path+cat_mat.specular_highlight_texname;
						set_mat.map_roughness = g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						);
					}
					else
					{
						set_mat.map_roughness = g_texture_manager.white_16x16;
					}

					sub_meshes[sub_mesh_id].material_using.maps_set = true;
				}
				
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// 遍历每个面的顶点
				for (size_t v = 0; v < fv; v++) 
				{
					vertex_standard one_vertex{};

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

					
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];

						one_vertex.normal = {
							nx,
							ny,
							nz
						};
					}
					

					if (idx.texcoord_index >= 0) 
					{
						tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
						tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

						one_vertex.uv0 = {
							tx,
							1.0f - ty
						};
					}

					// obj no vertex color.
					one_vertex.color = {1.0f,1.0f,1.0f};

					if (uniqueVertices.count(one_vertex) == 0) 
					{
						uniqueVertices[one_vertex] = static_cast<uint32_t>(vertices.size());

						vertices.push_back(one_vertex);

						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.x);
						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.y);
						raw_data.get_stream(vertex_attribute::pos).data.push_back(one_vertex.pos.z);

						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.x);
						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.y);
						raw_data.get_stream(vertex_attribute::color).data.push_back(one_vertex.color.z);

						raw_data.get_stream(vertex_attribute::uv0).data.push_back(one_vertex.uv0.x);
						raw_data.get_stream(vertex_attribute::uv0).data.push_back(one_vertex.uv0.y);
					}

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