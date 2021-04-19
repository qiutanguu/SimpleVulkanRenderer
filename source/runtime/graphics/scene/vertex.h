#pragma once
#include "../vk/vk_common.h"
#include "../vk/vk_device.h"
#include "../vk/vk_vertex_buffer.h"
#include <array>

namespace flower{ namespace graphics{

	// vertex���ڴ沼����Ӧ�ú�shader input����һ�¡�
	// ��һ��vertex�����ڶ��shader��ʹ�ã���ͬ��shader input���ܲ�ͬ��
	// ���Ӧ�ð�ÿ���������������ڴ�洢������
	// ��shader����ʱ��ѡ��ƴ�ӡ�

	// gltf2.0 bin ��ʽ���޳��ظ����㣬��������ڴ˴��ٴ��޳���
	struct vertex_data_stream
	{
		vertex_data_stream() = default;
		vertex_data_stream(const vertex_attribute& in) : type(in) {  }
		bool has_set() { return data.size() > 0; }
		std::vector<float> data = {};
		vertex_attribute type;
	};

	class vertex_raw_data
	{
	public:
		vertex_raw_data();
		~vertex_raw_data(){ pack_datas.clear(); }

		int32_t vertex_count();

		// ��������� attribute��������Ӧ��������
		std::vector<float> pack_type_stream(std::vector<vertex_attribute> type_composite)
		{
			std::vector<float> ret = {};
			pack_data_inner(type_composite,ret);
			return ret;
		}

		vertex_data_stream& get_stream(const vertex_attribute& in)
		{
			auto index = static_cast<uint32_t>(in);
			return pack_datas[index];
		}

	private:
		std::unordered_map<uint32_t,vertex_data_stream> pack_datas;
		void pack_data_inner(std::vector<vertex_attribute> type_composite,std::vector<float>& inout);
	};
}}