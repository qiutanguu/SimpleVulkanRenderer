#include "shader_manager.h"

namespace flower{ namespace graphics{

	shader_manager g_shader_manager = {};

	void shader_manager::create()
	{
		check_init();

	/// graphics shader
		texture_map_shader = vk_shader_mix::create(device,false,
			"data/shader/spir-v/texture_vert.spv",
			"data/shader/spir-v/texture_frag.spv"
		);

		gbuffer_shader = vk_shader_mix::create(device,false,
			"data/shader/spir-v/gbuffer_vert.spv",
			"data/shader/spir-v/gbuffer_frag.spv"
		);

		lighting_shader = vk_shader_mix::create(device,false,
			"data/shader/spir-v/lighting_vert.spv",
			"data/shader/spir-v/lighting_frag.spv"
		);

		tonemapper_shader = vk_shader_mix::create(device,false,
			"data/shader/spir-v/lighting_vert.spv",
			"data/shader/spir-v/tonemapper_frag.spv"
		);

		shadowdepth_shader = vk_shader_mix::create(device,false,
			"data/shader/spir-v/shadowdepth_vert.spv",
			"data/shader/spir-v/shadowdepth_frag.spv"
		);


	/// ui shader
		ui_vertex_shader = vk_shader_module::create(device,
			"data/shader/spir-v/uioverlay_vert.spv", 
			VK_SHADER_STAGE_VERTEX_BIT
		);

		ui_fragment_shader = vk_shader_module::create(device,
			"data/shader/spir-v/uioverlay_frag.spv", 
			VK_SHADER_STAGE_FRAGMENT_BIT
		);

	/// compute shader
		comp_edge_detect = vk_shader_mix::create(device,false,nullptr,nullptr,nullptr,
			"data/shader/spir-v/edge_detect_comp.spv",nullptr,nullptr);
	}

	void shader_manager::check_init()
	{
		ASSERT(has_init,"g_shader_manager…–Œ¥≥ı ºªØ£°");
	}

} }