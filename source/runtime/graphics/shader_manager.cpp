#include "shader_manager.h"

namespace flower{ namespace graphics{

	shader_manager g_shader_manager = {};

	void shader_manager::create()
	{
		check_init();

		texture_map_shader = vk_shader_mix::create(device,false,
			"data\\shader\\compiler\\spv\\texture_vert.spv",
			"data\\shader\\compiler\\spv\\texture_frag.spv"
		);

	}

	void shader_manager::check_init()
	{
		ASSERT(has_init,"g_shader_manager��δ��ʼ����");
	}

} }