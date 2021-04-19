#include "vertex.h"

namespace flower{ namespace graphics{
	
#define PACK_INSERT(Type) \
	vertex_data_stream data_string##Type(vertex_attribute::##Type);\
	uint32_t index##Type = static_cast<uint32_t>(vertex_attribute::##Type);\
	pack_datas.insert(std::make_pair(index##Type,data_string##Type))

	vertex_raw_data::vertex_raw_data()
	{
		pack_datas.clear();
		PACK_INSERT(pos);
		PACK_INSERT(uv0);
		PACK_INSERT(uv1);
		PACK_INSERT(normal);
		PACK_INSERT(tangent);
		PACK_INSERT(color);
		PACK_INSERT(alpha);
		PACK_INSERT(skin_weight);
		PACK_INSERT(skin_index);
		PACK_INSERT(skin_pack);
		PACK_INSERT(instance_float);
		PACK_INSERT(instance_vec2);
		PACK_INSERT(instance_vec3);
		PACK_INSERT(instance_vec4);
		PACK_INSERT(uv2);
		PACK_INSERT(uv3);
		PACK_INSERT(uv4);
		PACK_INSERT(uv5);
		PACK_INSERT(uv6);
	}

	int32_t vertex_raw_data::vertex_count()
	{
		return int32_t(pack_datas[(uint32_t)vertex_attribute::pos].data.size()) / vertex_attribute_count(vertex_attribute::pos);
	}

	int32_t get_per_vertex_size(std::vector<vertex_attribute> type_composite)
	{
		int32_t size = 0;
		for(auto& type : type_composite)
		{
			size += vertex_attribute_count(type);
		}
		return size;
	}
	
	void graphics::vertex_raw_data::pack_data_inner(
		std::vector<vertex_attribute> type_composite,
		std::vector<float>& inout)
	{
		int32_t per_vertex_size = get_per_vertex_size(type_composite);
		inout.resize(per_vertex_size * vertex_count());

		int32_t working_point = 0;
		for(int32_t vertex_index = 0; vertex_index < vertex_count(); vertex_index++)
		{
			for(auto& type : type_composite)
			{
				int32_t attri_size = vertex_attribute_count(type);
				
				if(pack_datas[(uint32_t)type].has_set())
				{
					// 填充数据
					for(int32_t size_bit = 0; size_bit < attri_size; size_bit++)
					{
						auto curr_type_raw_data_bit = vertex_index * attri_size + size_bit;
						inout[working_point] = pack_datas[(uint32_t)type].data[curr_type_raw_data_bit];
						working_point++;
					}
				}
				else
				{
					// 若对应类型未设置则填充0
					for(int32_t size_bit = 0; size_bit < attri_size; size_bit++)
					{
						inout[working_point] = 0.0f;
						working_point++;
					}
				}
			}
		}
	}
}}