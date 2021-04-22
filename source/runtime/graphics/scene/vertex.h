#pragma once
#include "../vk/vk_common.h"
#include "../vk/vk_device.h"
#include "../vk/vk_vertex_buffer.h"
#include <array>

namespace flower{ namespace graphics{

	// vertex 在内存布局上应该和 shader input 保持一致。
	// 但一套vertex可能在多个shader上使用，不同的 shader input 可能不同。
	// 因此应该把每种类型以连续的内存存储起来。
	// 在shader布局时在选择拼接。

	// gltf2.0 bin 格式会剔除重复顶点，因此无需在此处再次剔除。
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

		// 根据输入的 attribute 组合输出对应的数据流
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
		
		// 释放内存中的数据
		void release_cpu_data()
		{
			pack_datas.clear();
		}

	private:
		std::unordered_map<uint32_t,vertex_data_stream> pack_datas;
		void pack_data_inner(std::vector<vertex_attribute> type_composite,std::vector<float>& inout);
	};
}}