#include "shader_manager.h"

namespace flower{ namespace graphics{

	shader_manager g_shader_manager = {};

	void shader_manager::create()
	{
		check_init();

		texture_map_shader = vk_shader_mix::create(device,false,
			"data/shader/compiler/spv/texture_vert.spv",
			"data/shader/compiler/spv/texture_frag.spv"
		);

		gbuffer_shader = vk_shader_mix::create(device,false,
			"data/shader/compiler/spv/gbuffer_vert.spv",
			"data/shader/compiler/spv/gbuffer_frag.spv"
		);

		lighting_shader = vk_shader_mix::create(device,false,
			"data/shader/compiler/spv/lighting_vert.spv",
			"data/shader/compiler/spv/lighting_frag.spv"
		);

		tonemapper_shader = vk_shader_mix::create(device,false,
			"data/shader/compiler/spv/lighting_vert.spv",
			"data/shader/compiler/spv/tonemapper_frag.spv"
		);
	}

	void shader_manager::check_init()
	{
		ASSERT(has_init,"g_shader_manager…–Œ¥≥ı ºªØ£°");
	}

} }