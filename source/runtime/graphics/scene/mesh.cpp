#include "mesh.h"
#include "../texture_manager.h"
#include <graphics/shader_manager.h>
#include "../pass/texture_pass.h"
#include "../pass/gbuffer_pass.h"
#include "core/timer.h"

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

		// ���е�ģ�;������ʱ��Ĭ��ģ�;���
		model = glm::rotate(glm::mat4(1.0f),glm::radians(0.0f),glm::vec3(-1.0f,0.0f,0.0f));

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

	void mesh::load_obj_mesh(
		std::string path,
		std::string mat_path)
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

	void mesh::register_renderpass(std::shared_ptr<vk_renderpass> pass,std::shared_ptr<vk_shader_mix> shader,bool reload_vertex_buf)
	{
		for(auto& submesh:sub_meshes)
		{
			submesh.register_renderpass(pass->type,device,pass->render_pass,pool);
		}

		if(reload_vertex_buf)
		{
			// �ϴ�render pass��Ӧ�Ķ���buffer
			vertex_bufs[pass->type] = vk_vertex_buffer::create(
				device,
				pool,
				raw_data.pack_type_stream(shader->per_vertex_attributes),
				shader->per_vertex_attributes
			);
		}

		has_registered[pass->type] = true;
	}

	void meshes_manager::initialize(vk_device* indevice,VkCommandPool inpool)
	{
		pool = inpool;
		device = indevice;

		// sponza ������ص��ڴ���
		sponza_mesh = std::make_shared<mesh>(device,pool);
		sponza_mesh->load_obj_mesh("data/model/sponza/sponza.obj","");
	}

	// �ͷż��ص�cpu�е���������
	void meshes_manager::release_cpu_mesh_data()
	{
		sponza_mesh->raw_data.release_cpu_data();
	}

} }