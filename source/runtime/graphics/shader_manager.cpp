#include "shader_manager.h"

namespace flower{ namespace graphics{

	shader_manager g_shader_manager = {};

	void shader_manager::create()
	{
		check_init();

		texture_map_shader = vk_shader_mix::create(device,false,
			"data/model/sponza/vert.spv",
			"data/model/sponza/frag.spv"
		);

	}

	void shader_manager::check_init()
	{
		ASSERT(has_init,"g_shader_manager…–Œ¥≥ı ºªØ£°");
	}

} }