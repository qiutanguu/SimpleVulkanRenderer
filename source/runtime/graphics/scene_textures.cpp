#include "scene_textures.h"

namespace flower { namespace graphics{
	
	scene_textures g_scene_textures = {};

	void scene_textures::create()
	{
		check_init();

		scene_depth_stencil = vk_texture::create_depth_no_msaa(device,swapchain);
		scene_depth_stencil->update_sampler(
			sampler_layout::nearset_clamp()
		);

		scene_color = vk_texture::create_color_attachment(
			device,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			swapchain
		);
		scene_color->update_sampler(
			sampler_layout::nearset_clamp()
		);

		normal_worldspace = vk_texture::create_color_attachment(
			device,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			swapchain
		);
		normal_worldspace->update_sampler(
			sampler_layout::nearset_clamp()
		);

		position_worldspace = vk_texture::create_color_attachment(
			device,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			swapchain
		);
		position_worldspace->update_sampler(
			sampler_layout::nearset_clamp()
		);

		basecolor = vk_texture::create_color_attachment(
			device,
			VK_FORMAT_R8G8B8A8_UNORM,
			swapchain
		);
		basecolor->update_sampler(
			sampler_layout::nearset_clamp()
		);

	}

}}
