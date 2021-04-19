#include "scene_textures.h"

namespace flower { namespace graphics{
	
	scene_textures g_scene_textures = {};

	void scene_textures::create()
	{
		check_init();
		scene_depth_stencil = vk_texture::create_depth_no_msaa(device,swapchain);
	}

}}
