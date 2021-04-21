#include "scene_textures.h"

namespace flower { namespace graphics{
	
	scene_textures g_scene_textures = {};

	void scene_textures::create()
	{
		check_init();
		scene_depth_stencil = vk_texture::create_depth_no_msaa(device,swapchain);

		normal_worldspace.resize(swapchain->get_imageViews().size());
		normal_worldspace = vk_texture::create_rt(
			device,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			swapchain,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		);

		position_worldspace = vk_texture::create_rt(
			device,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			swapchain,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		);

		basecolor = vk_texture::create_rt(
			device,
			VK_FORMAT_R8G8B8A8_UNORM,
			swapchain,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		);

	}

}}
