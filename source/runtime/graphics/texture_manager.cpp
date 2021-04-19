#include "texture_manager.h"

namespace flower { namespace graphics{

	texture_manager g_texture_manager = {};

	uint32_t texture_manager::load_texture_mipmap(
		VkFormat texture_format,
		const sampler_layout& layout,
		std::string texture_path)
	{
		check_init();

		if(texture_map.count(texture_path) > 0)
			return texture_map[texture_path];

		auto res = vk_texture::create_2d_mipmap(device,pool,texture_format,texture_path);

		res->update_sampler(
			layout
		);

		texture_map.insert(std::make_pair(texture_path,current_id));
		textures.push_back(res);
		current_id ++;

		return current_id-1;
	}

	void texture_manager::check_init()
	{
		ASSERT(has_init,"texture manager ÉĞÎ´³õÊ¼»¯£¡");
	}



} }