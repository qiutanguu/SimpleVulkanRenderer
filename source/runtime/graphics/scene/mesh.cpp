#include "mesh.h"
#include "../texture_manager.h"
#include <graphics/shader_manager.h>
#include "../pass/texture_pass.h"
#include "../pass/gbuffer_pass.h"
#include "core/timer.h"
#include "../pass/shadowdepth_pass.h"
#include "asset_system/asset_pmx.h"

// ��׼����
struct vertex_standard
{
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(0.0f);
	glm::vec2 uv0 = glm::vec2(0.0f);
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec4 tangent = glm::vec4(0.0f);

	bool operator==(const vertex_standard& other) const
	{
		return pos==other.pos&&
			color==other.color&&
			uv0==other.uv0&&
			normal==other.normal&&
			tangent==other.tangent;
	}
};

namespace std
{
	template<> struct hash<vertex_standard>
	{
		size_t operator()(vertex_standard const& vertex) const
		{
			return
				((hash<glm::vec3>()(vertex.pos)^
					(hash<glm::vec3>()(vertex.color)<<1))>>1)^
				(hash<glm::vec2>()(vertex.uv0)<<1)^
				(hash<glm::vec3>()(vertex.normal)<<1)^
				(hash<glm::vec4>()(vertex.tangent)>>1);
		}
	};
}

namespace flower{ namespace graphics{
	meshes_manager g_meshes_manager = {};

	void sub_mesh::draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t passtype)
	{
		ASSERT(passtype<renderpass_type::max_index,"render pass typeԽ�硣");

		mat_map[passtype]->pipeline->bind(*cmd_buf);

		vkCmdBindDescriptorSets(
			*cmd_buf,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mat_map[passtype]->pipeline->layout,
			0,
			1,
			mat_map[passtype]->descriptor_set->descriptor_sets.data(),
			0,
			nullptr
		);

		index_buf->bind_and_draw(*cmd_buf);
	}

	// ��ÿ�� renderpass ��Ӧ��ע���Ӧ�� shader material
	void sub_mesh::register_renderpass(
		int32_t passtype,
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool
	){
		ASSERT(passtype < renderpass_type::max_index,"render pass typeԽ�硣");
		
		// ����ÿ��renderpass����Ҫ�����������ǵ�descriptor����
		if(passtype == renderpass_type::texture_pass)
		{
			mat_map[passtype] = material_texture::create(
				indevice,
				in_renderpass,
				in_pool,
				texture_ids[texture_id_type::diffuse],
				model
			);
		}
		else if(passtype == renderpass_type::gbuffer_pass)
		{
			mat_map[passtype] = material_gbuffer::create(
				indevice,
				in_renderpass,
				in_pool,
				texture_ids,
				model
			);
		}
		else if(passtype == renderpass_type::shadowdepth_pass)
		{
			mat_map[passtype] = material_shadowdepth::create(
				indevice,
				in_renderpass,
				in_pool,
				texture_ids,
				model
			);
		}
		else if(passtype==renderpass_type::gbuffer_character_pass)
		{
			mat_map[passtype] = material_gbuffer_character::create(
				indevice,
				in_renderpass,
				in_pool,
				texture_ids,
				model
			);
		}
		else
		{
			LOG_VULKAN_FATAL("δ�����pass����{0}��",passtype);
		}

		has_registered[passtype] = true;
	}

	void mesh::draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t pass_type)
	{
		ASSERT(pass_type < renderpass_type::max_index,"render pass typeԽ�硣");

		// �����еĶ��㻺��
		vertex_bufs[pass_type]->bind(*cmd_buf);

		for (auto& submesh : sub_meshes)
		{
			submesh.draw(cmd_buf,pass_type);
		}
	}

	void mesh::load_pmx_mesh(std::string path,const glm::mat4& model)
	{
		flower::asset::PMXFile file{};
		flower::asset::ReadPMXFile(&file,path.c_str());

		// ͬ��һ�����ʷ���һ��submesh
		sub_meshes.resize(file.m_materials.size());
		for(auto& it : sub_meshes)
		{
			it.model = model;
		}

		auto end_pos = path.find_last_of("/\\");

		std::string pmx_folder_path;
		if(end_pos!=std::string::npos)
		{
			pmx_folder_path = path.substr(0,end_pos);
		}

		pmx_folder_path += "/";

		// ���raw data
		auto& pos_data = raw_data.get_stream(vertex_attribute::pos).data;
		auto& normal_data = raw_data.get_stream(vertex_attribute::normal).data;
		auto& uv0_data = raw_data.get_stream(vertex_attribute::uv0).data;

		pos_data.resize(file.m_vertices.size() * 3);
		normal_data.resize(file.m_vertices.size() * 3);
		uv0_data.resize(file.m_vertices.size() * 2);

		// ���raw data����
		for(auto i_v = 0; i_v < file.m_vertices.size(); i_v ++)
		{
			auto pos_index = 3 * i_v;
			auto normal_index = 3 * i_v;
			auto uv0_index = 2 * i_v;

			pos_data[pos_index] = file.m_vertices[i_v].m_position.x;
			pos_data[pos_index + 1] = file.m_vertices[i_v].m_position.y;
			pos_data[pos_index + 2] = file.m_vertices[i_v].m_position.z;

			normal_data[normal_index] = file.m_vertices[i_v].m_normal.x;
			normal_data[normal_index + 1] = file.m_vertices[i_v].m_normal.y;
			normal_data[normal_index + 2] = file.m_vertices[i_v].m_normal.z;

			uv0_data[uv0_index] = file.m_vertices[i_v].m_uv.x;
			uv0_data[uv0_index + 1] = file.m_vertices[i_v].m_uv.y;
		}

		// �������е�����
		// pmx ���������� srgb 233
		for(auto mat_i = 0; mat_i<file.m_materials.size(); mat_i++)
		{
			auto& set_mat = sub_meshes[mat_i].texture_ids;
			auto& process_mat = file.m_materials[mat_i];

			// ʹ�þ���·����Ϊ���
			if(process_mat.m_textureIndex>=0)
			{
				auto base_color_path = pmx_folder_path + file.m_textures[process_mat.m_textureIndex].m_textureName;
				set_mat.push_back(g_texture_manager.load_texture_mipmap(
					VK_FORMAT_R8G8B8A8_SRGB,
					sampler_layout::linear_repeat(),
					base_color_path
				));
			}
			else
			{
				set_mat.push_back(g_texture_manager.checkboard);
			}
			
			if(process_mat.m_toonTextureIndex>=0)
			{
				auto toon_tex_path = pmx_folder_path + file.m_textures[process_mat.m_toonTextureIndex].m_textureName;
				set_mat.push_back(g_texture_manager.load_texture_mipmap(
					VK_FORMAT_R8G8B8A8_SRGB,
					sampler_layout::linear_repeat(),
					toon_tex_path
				));
			}
			else
			{
				set_mat.push_back(g_texture_manager.checkboard);
			}

			if(process_mat.m_sphereTextureIndex >= 0)
			{
				auto sphere_tex_path = pmx_folder_path + file.m_textures[process_mat.m_sphereTextureIndex].m_textureName;
				set_mat.push_back(g_texture_manager.load_texture_mipmap(
					VK_FORMAT_R8G8B8A8_SRGB,
					sampler_layout::linear_repeat(),
					sphere_tex_path
				));
			}
			else
			{
				set_mat.push_back(g_texture_manager.checkboard);
			}
		}

		

		// �ֱ���ÿһ������
		int32_t start_face_point = 0;
		int32_t idex = 0;

		for(auto& mat : file.m_materials)
		{
			auto& processing_submesh = sub_meshes[idex];
			idex ++;
			auto face_num = mat.m_numFaceVertices / 3;
			processing_submesh.indices.resize(mat.m_numFaceVertices);

			int32_t vertex_stamp = 0;
			int32_t end_face_point = start_face_point + face_num;
			for(auto face_id = start_face_point;face_id < end_face_point; face_id++)
			{
				auto& face = file.m_faces[face_id];
				for(auto vertex : face.m_vertices)
				{
					processing_submesh.indices[vertex_stamp] = vertex;
					vertex_stamp ++;
				}
			}
			start_face_point = end_face_point;
		}

		// �ϴ����򻺳�
		for(auto& submesh : sub_meshes)
		{
			auto& indices = submesh.indices;
			submesh.index_buf = vk_index_buffer::create(device,pool,indices);
		}
	}

	void mesh::load_obj_mesh(
		std::string path,
		std::string mat_path,const glm::mat4& model)
	{
		double time_stamp = flower::global_timer::get_timer_second();
		
		LOG_IO_TRACE("��ʼ����{0}����",path);

		std::string mtl_search_path;
		size_t pos = path.find_last_of("/\\");
		std::string mesh_name;
		if(pos!=std::string::npos)
		{
			mtl_search_path = path.substr(0,pos);
			mesh_name = path.substr(pos);
		}

		mtl_search_path += "/";

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

		time_stamp = flower::global_timer::get_timer_second() - time_stamp;
		LOG_IO_INFO("����{0}���񻨷�{1}�룡",path,time_stamp);
		LOG_IO_TRACE("��ʼ����{0}����",mesh_name);

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		std::unordered_map<vertex_standard, uint32_t> uniqueVertices{};
		std::vector<vertex_standard> vertices;

		sub_meshes.resize(materials.size());

		for(auto& it : sub_meshes)
		{
			it.model = model;
		}

		// ����ÿ������
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			// ����ÿ����
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
			{
				auto sub_mesh_id = shapes[s].mesh.material_ids[f];

				// �������е�����
				if(sub_meshes.size() > 0 && sub_meshes[sub_mesh_id].texture_ids.size() < 1)
				{
					auto& set_mat = sub_meshes[sub_mesh_id].texture_ids;
					auto& cat_mat = materials[sub_mesh_id];

					// ��texture_id_type����ÿ����������

					// 0 ��diffuse
					if(cat_mat.diffuse_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.diffuse_texname;
						set_mat.push_back(g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_SRGB,
							sampler_layout::linear_repeat(),
							texpath
						));
					}
					else
					{
						set_mat.push_back(g_texture_manager.checkboard);
					}

					// 1 ��mask
					if(cat_mat.alpha_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.alpha_texname;
						set_mat.push_back(g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						));
					}
					else
					{
						set_mat.push_back(g_texture_manager.white_16x16);
					}

					// 2 ��metalic
					if(cat_mat.specular_texname!="")
					{
						auto texpath = mtl_search_path + cat_mat.specular_texname;
						set_mat.push_back(g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						));
					}
					else
					{
						set_mat.push_back(g_texture_manager.black_16x16);
					}

					// 3 ��normal map
					if(cat_mat.bump_texname!="")
					{
						auto texpath = mtl_search_path+cat_mat.bump_texname;
						set_mat.push_back(g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						));
					}
					else
					{
						set_mat.push_back(g_texture_manager.default_normal);
					}

					// 4 ��roughness map
					if(cat_mat.specular_highlight_texname!="")
					{
						auto texpath = mtl_search_path+cat_mat.specular_highlight_texname;
						set_mat.push_back(g_texture_manager.load_texture_mipmap(
							VK_FORMAT_R8G8B8A8_UNORM,
							sampler_layout::linear_repeat(),
							texpath
						));
					}
					else
					{
						set_mat.push_back(g_texture_manager.white_16x16);
					}
				}
				
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				ASSERT(fv == 3,"��֧�������棡");
				
				// �����ζ�����������
				std::array<glm::vec3,3> v_pos = { glm::vec3(0.0f),glm::vec3(0.0f),glm::vec3(0.0f) };
				std::array<glm::vec3,3> v_normal = {glm::vec3(0.0f),glm::vec3(0.0f),glm::vec3(0.0f) };
				std::array<glm::vec2,3> v_uv  = {glm::vec2(0.0f),glm::vec2(0.0f),glm::vec2(0.0f) };

				// ����ÿ����Ķ���
				for (size_t v = 0; v < fv; v++) 
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

					v_pos[v] = glm::vec3(vx,vy,vz);

					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];

						v_normal[v] = {
							nx,
							ny,
							nz
						};
					}
					
					if (idx.texcoord_index >= 0) 
					{
						tinyobj::real_t tx = attrib.texcoords[ 2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[ 2 * size_t(idx.texcoord_index) + 1];

						v_uv[v] = {
							tx,
							1- ty
						};
					}
				}
				
				// �ռ����
				glm::vec3 deltaPos1 = v_pos[1] - v_pos[0];
				glm::vec3 deltaPos2 = v_pos[2] - v_pos[0];

				// uv ����
				glm::vec2 deltaUV1 = v_uv[1] - v_uv[0];
				glm::vec2 deltaUV2 = v_uv[2] - v_uv[0];

				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
				tangent = glm::normalize(tangent);
				glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
				bitangent = glm::normalize(bitangent);
				
				for (size_t v = 0; v < fv; v++) 
				{
					vertex_standard one_vertex{};

					one_vertex.pos = v_pos[v];
					one_vertex.normal = glm::normalize(v_normal[v]);
					one_vertex.uv0 = v_uv[v];

					one_vertex.tangent = glm::vec4(
						tangent.x,
						tangent.y,
						tangent.z,
						1
					);

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

						raw_data.get_stream(vertex_attribute::normal).data.push_back(one_vertex.normal.x);
						raw_data.get_stream(vertex_attribute::normal).data.push_back(one_vertex.normal.y);
						raw_data.get_stream(vertex_attribute::normal).data.push_back(one_vertex.normal.z);

						raw_data.get_stream(vertex_attribute::tangent).data.push_back(one_vertex.tangent.x);
						raw_data.get_stream(vertex_attribute::tangent).data.push_back(one_vertex.tangent.y);
						raw_data.get_stream(vertex_attribute::tangent).data.push_back(one_vertex.tangent.z);
						raw_data.get_stream(vertex_attribute::tangent).data.push_back(one_vertex.tangent.w);
					}

					if( sub_meshes.size() > 0 )
					{
						auto& subMesh_indices = sub_meshes[sub_mesh_id].indices;
						subMesh_indices.push_back(uniqueVertices[one_vertex]);
					}
				}

				index_offset += fv;
			}
		}

		time_stamp = flower::global_timer::get_timer_second()-time_stamp;
		LOG_IO_INFO("����{0}���񻨷�{1}�룡",mesh_name,time_stamp);
		
		// �ϴ����򻺳�
		for(auto& submesh:sub_meshes)
		{
			auto& indices = submesh.indices;
			submesh.index_buf = vk_index_buffer::create(device,pool,indices);
		}
	}

	void mesh::register_renderpass(std::shared_ptr<vk_renderpass> pass,std::shared_ptr<vk_shader_mix> shader,uint32_t pass_type,bool reload_vertex_buf)
	{
		for(auto& submesh:sub_meshes)
		{
			submesh.register_renderpass(pass_type ,device,pass->render_pass,pool);
		}

		if(reload_vertex_buf)
		{
			// �ϴ�render pass��Ӧ�Ķ���buffer
			vertex_bufs[pass_type] = vk_vertex_buffer::create(
				device,
				pool,
				raw_data.pack_type_stream(shader->per_vertex_attributes),
				shader->per_vertex_attributes
			);
		}

		has_registered[pass_type] = true;
	}

	void meshes_manager::initialize(vk_device* indevice,VkCommandPool inpool)
	{
		pool = inpool;
		device = indevice;

		// sponza ������ص��ڴ���
		auto native = get_mesh_transform(glm::vec3(1.0f),glm::vec3(0.0f),glm::vec3(0.0f));

		sponza_mesh = std::make_shared<mesh>(device,pool);
		sponza_mesh->load_obj_mesh("data/model/sponza/sponza.obj","",native);

		auto miku_scale = get_mesh_transform(glm::vec3(10.0f),glm::vec3(50.0f,0,0),glm::vec3(0,glm::pi<float>() * -0.5f,0));
		miku_mesh = std::make_shared<mesh>(device,pool);
		miku_mesh->load_pmx_mesh("data/model/HCMiku v3 ver1.00/HCMiku v3.pmx",miku_scale);
	}

	// �ͷż��ص�cpu�е���������
	void meshes_manager::release_cpu_mesh_data()
	{
		sponza_mesh->raw_data.release_cpu_data();
		miku_mesh->raw_data.release_cpu_data();
	}

	void quad_mesh::initialize(vk_device* in_device,VkCommandPool in_pool)
	{
		vertex_buffer.reset();
		vertex_buffer = vk_vertex_buffer::create(in_device,in_pool,vertices,{vertex_attribute::pos,vertex_attribute::uv0});
		
		index_buffer.reset();
		index_buffer = vk_index_buffer::create(in_device,in_pool,indices);
	}

	glm::mat4 meshes_manager::get_mesh_transform(const glm::vec3& scale,const glm::vec3& pos, const glm::vec3& rotate)
	{
		glm::mat4 res = glm::mat4(1.0f);

		res = glm::scale(
			res,
			scale
		);

		res = glm::translate(
			res,
			pos
		);

		res = glm::rotate(
			res,
			rotate.x,
			glm::vec3(1.0f,0,0)
		);

		res = glm::rotate(
			res,
			rotate.y,
			glm::vec3(0,1.0f,0)
		);

		res = glm::rotate(
			res,
			rotate.z,
			glm::vec3(0,0,1.0f)
		);

		return res * global_ident_mat4_model;
	}

}}