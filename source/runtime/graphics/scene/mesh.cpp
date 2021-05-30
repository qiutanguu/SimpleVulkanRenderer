#include "mesh.h"
#include "../texture_manager.h"
#include <graphics/shader_manager.h>
#include "../pass/texture_pass.h"
#include "../pass/gbuffer_pass.h"
#include "core/timer.h"
#include "../pass/shadowdepth_pass.h"
#include "asset_system/asset_pmx.h"
#include "core/unicode.hpp"

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
		ASSERT(passtype<renderpass_type::max_index,"render pass type越界。");

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

	// 对每种 renderpass 都应该注册对应的 shader material
	void sub_mesh::register_renderpass(
		int32_t passtype,
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool
	){
		ASSERT(passtype < renderpass_type::max_index,"render pass type越界。");
		
		// 对于每种renderpass，需要特殊设置它们的descriptor创建
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
			LOG_VULKAN_FATAL("未处理的pass类型{0}！",passtype);
		}

		has_registered[passtype] = true;
	}

	void mesh::draw(std::shared_ptr<vk_command_buffer> cmd_buf,int32_t pass_type)
	{
		ASSERT(pass_type < renderpass_type::max_index,"render pass type越界。");

		// 绑定所有的顶点缓冲
		vertex_bufs[pass_type]->bind(*cmd_buf);

		for (auto& submesh : sub_meshes)
		{
			submesh.draw(cmd_buf,pass_type);
		}
	}

	void mesh::load_obj_mesh(
		std::string path,
		std::string mat_path,const glm::mat4& model)
	{
		double time_stamp = flower::global_timer::get_timer_second();
		
		LOG_IO_TRACE("开始加载{0}网格！",path);

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
				LOG_IO_FATAL("obj网格读取错误：{0}",reader.Error());
			}
		}

		if (!reader.Warning().empty()) 
		{
			LOG_IO_WARN("obj网格读取警告：{0}",reader.Warning());
		}

		time_stamp = flower::global_timer::get_timer_second() - time_stamp;
		LOG_IO_INFO("加载{0}网格花费{1}秒！",path,time_stamp);
		LOG_IO_TRACE("开始解析{0}网格！",mesh_name);

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

		// 遍历每个网格
		for (size_t s = 0; s < shapes.size(); s++) 
		{
			// 遍历每个面
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
			{
				auto sub_mesh_id = shapes[s].mesh.material_ids[f];

				// 加载所有的纹理！
				if(sub_meshes.size() > 0 && sub_meshes[sub_mesh_id].texture_ids.size() < 1)
				{
					auto& set_mat = sub_meshes[sub_mesh_id].texture_ids;
					auto& cat_mat = materials[sub_mesh_id];

					// 按texture_id_type加载每种类型纹理

					// 0 是diffuse
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

					// 1 是mask
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

					// 2 是metalic
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

					// 3 是normal map
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

					// 4 是roughness map
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

				ASSERT(fv == 3,"仅支持三角面！");
				
				// 三角形顶点坐标数据
				std::array<glm::vec3,3> v_pos = { glm::vec3(0.0f),glm::vec3(0.0f),glm::vec3(0.0f) };
				std::array<glm::vec3,3> v_normal = {glm::vec3(0.0f),glm::vec3(0.0f),glm::vec3(0.0f) };
				std::array<glm::vec2,3> v_uv  = {glm::vec2(0.0f),glm::vec2(0.0f),glm::vec2(0.0f) };

				// 遍历每个面的顶点
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
				
				// 空间距离
				glm::vec3 deltaPos1 = v_pos[1] - v_pos[0];
				glm::vec3 deltaPos2 = v_pos[2] - v_pos[0];

				// uv 距离
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
		LOG_IO_INFO("解析{0}网格花费{1}秒！",mesh_name,time_stamp);
		
		// 上传次序缓冲
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
			// 上传render pass对应的顶点buffer
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

		// sponza 网格加载到内存中
		auto native = get_mesh_transform(glm::vec3(1.0f),glm::vec3(0.0f),glm::vec3(0.0f));

		miku_mesh = std::make_shared<mesh_mmd>(device,pool);
		miku_mesh->initialize("data/model/HCMiku v3 ver1.00/HCMiku v3.pmx","data/model/HCMiku v3 ver1.00/dance.vmd");

		sponza_mesh = std::make_shared<mesh>(device,pool);
		sponza_mesh->load_obj_mesh("data/model/sponza/sponza.obj","",native);
	}

	// 释放加载到cpu中的网格数据
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

	bone_processor_mmd::bone_processor_mmd(asset::PMXFile* file)
	{
		list.resize(file->m_bones.size());
		for(auto i = 0; i<list.size(); i++)
		{
			list[i] = new bone_node_mmd(file->m_bones[i]);
			list[i]->index = i;
		}

		// 建树。
		for(auto* node : list)
		{
			if(node->bone.m_parentBoneIndex!=-1)
			{
				// 设定好父子关系。
				node->parent = list[node->bone.m_parentBoneIndex];
				list[node->bone.m_parentBoneIndex]->childs.push_back(node);
			}
			else
			{
				node->parent = nullptr;
				roots.push_back(node);
			}
		}
	}

	bone_processor_mmd::~bone_processor_mmd()
	{
		for(auto* node : list)
		{
			delete node;
		}
	}

	bone_node_mmd* bone_processor_mmd::operator[](std::u16string name)
	{
		for(auto* node : list)
		{
			if(node->bone.m_u16name == name)
			{
				return node;
			}
		}
		return nullptr;
	}

	void mesh_mmd::initialize(std::string mesh_path,std::string vmd_path)
	{
		load_pmx_mesh(mesh_path);
		load_vmd_data(vmd_path);

		if(bone_manager!=nullptr)
		{
			delete bone_manager;
			bone_manager = nullptr;
		}
			
		// 创建骨骼层级树。
		bone_manager = new bone_processor_mmd(&miku_skeleton_mesh);
	}

	void mesh_mmd::load_pmx_mesh(std::string mesh_path)
	{
		this->miku_skeleton_mesh = {};
		auto& file = this->miku_skeleton_mesh;
		flower::asset::ReadPMXFile(&this->miku_skeleton_mesh,mesh_path.c_str());

		// 同样一个材质分配一个submesh
		sub_meshes.resize(this->miku_skeleton_mesh.m_materials.size());
		auto miku_scale = meshes_manager::get_mesh_transform(glm::vec3(10.0f),glm::vec3(50.0f,0,0),glm::vec3(0,glm::pi<float>() * -0.5f,0));
		for(auto& it : sub_meshes)
		{
			it.model = miku_scale;
		}

		auto end_pos = mesh_path.find_last_of("/\\");

		std::string pmx_folder_path;
		if(end_pos!=std::string::npos)
		{
			pmx_folder_path = mesh_path.substr(0,end_pos);
		}

		pmx_folder_path += "/";

		// 填充raw data
		auto& pos_data = raw_data.get_stream(vertex_attribute::pos).data;
		auto& normal_data = raw_data.get_stream(vertex_attribute::normal).data;
		auto& uv0_data = raw_data.get_stream(vertex_attribute::uv0).data;

		pos_data.resize(file.m_vertices.size() * 3);
		normal_data.resize(file.m_vertices.size() * 3);
		uv0_data.resize(file.m_vertices.size() * 2);

		// 填充raw data顶点
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

		// 加载所有的纹理
		// pmx 所有纹理都是 srgb 233
		for(auto mat_i = 0; mat_i<file.m_materials.size(); mat_i++)
		{
			auto& set_mat = sub_meshes[mat_i].texture_ids;
			auto& process_mat = file.m_materials[mat_i];

			// 使用绝对路径作为标记
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

		// 分别处理每一个材质
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

		// 上传次序缓冲
		for(auto& submesh : sub_meshes)
		{
			auto& indices = submesh.indices;
			submesh.index_buf = vk_index_buffer::create(device,pool,indices);
		}
	}

	bool animation_key_sort_greater(animation_key k0,animation_key k1)
	{
		return k0.time > k1.time;
	}

	bool animation_key_sort_less(animation_key k0,animation_key k1)
	{
		return k0.time < k1.time;
	}

	void mesh_mmd::load_vmd_data(std::string vmd_path)
	{
		this->dance_data = {};
		flower::asset::ReadVMDFile(&this->dance_data,vmd_path.c_str());

		// 注册所有的vmd骨骼
		for (auto i = 0; i < miku_skeleton_mesh.m_bones.size(); i++)
		{
			auto key_string = miku_skeleton_mesh.m_bones[i].m_u16name;
			int32_t key_int = flower::unicode::u16stringToHash(key_string);

			ASSERT(bone_animation_keys.find(key_int) == bone_animation_keys.end(),
				"重复的Bone {0}！",miku_skeleton_mesh.m_bones[i].m_name.c_str());

			bone_animation_keys.insert( std::make_pair(key_int,std::vector<animation_key>()));
		}

		// 为每个vmd骨骼都填入它们对应的所有关键帧
		for ( auto& motion : dance_data.m_motions)
		{
			int32_t key_int = flower::unicode::u16stringToHash(motion.boneName);
			std::vector<animation_key>& bone_vector = bone_animation_keys[key_int];

			animation_key akey(motion);
			bone_vector.push_back(akey);
		}

		// 对vmd关键帧排序
		for ( auto& keys : bone_animation_keys)
		{
			auto& bone_vector =  keys.second;
			std::sort(bone_vector.begin(),bone_vector.end(),animation_key_sort_less); // 降序排序
		}
	}

	// 
	void mesh_mmd::build_bone_matrices(float time,const std::u16string& name)
	{
		int32_t key_int = unicode::u16stringToHash(name);
		auto& bone_vector = bone_animation_keys[key_int];
		
		// 找到最近临近两帧
		auto near_frame = find_key(time,bone_vector);


		int32_t last_key = near_frame.first;
		int32_t next_key = near_frame.second;


	}

	std::pair<int32_t,int32_t> mesh_mmd::find_key(float time,const std::vector<animation_key>& keys)
	{
		// 处理三种退化情况
		if(keys.size()==0)
		{
			return std::make_pair(-1,-1);
		}

		if(keys.size()==1)
		{
			return std::make_pair(0,0);
		}

		if(keys.size()==2)
		{
			return std::make_pair(0,1);
		}

		float time_key = time * (float)vmd_frame_rate;

		int32_t end = (int32_t)keys.size() - 1;
		int32_t start = 0;

		while(true)
		{
			int32_t mid = (end + start) / 2;
			float mid_time = float(keys[mid].time) / float(vmd_frame_rate);

			if(time_key == mid_time)
			{
				return std::make_pair(mid,mid);
			}
			else if(end - start <= 1)
			{
				return std::make_pair(start,end);
			}
			else if(time_key < mid_time) // 在左边
			{
				end = mid;
			}
			else // 在右边
			{
				start = mid;
			}
		}

		return std::make_pair(-2,-2);
	}

}}