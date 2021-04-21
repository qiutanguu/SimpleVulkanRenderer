#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_texture.h"
#include "vk/vk_swapchain.h"

namespace flower{ namespace graphics{

	class scene_textures
	{
	public:
		scene_textures() { }

		~scene_textures()
		{
		}

		void initialize(vk_device* in_device,vk_swapchain* in_swapchain)
		{
			device = in_device;
			swapchain = in_swapchain;
			has_init = true;
			create();
		}

		void release()
		{
			position_worldspace.reset();
			normal_worldspace.reset();
			basecolor.reset();

			scene_depth_stencil.reset();
		}


	public:
		std::shared_ptr<vk_texture> position_worldspace; // VK_FORMAT_R16G16B16A16_SFLOAT
		std::shared_ptr<vk_texture> normal_worldspace; // VK_FORMAT_R16G16B16A16_SFLOAT
		std::shared_ptr<vk_texture> basecolor; // VK_FORMAT_R8G8B8A8_UNORM

		std::shared_ptr<vk_texture> scene_depth_stencil;

	private:
		bool has_init = false;
		void check_init() { ASSERT(has_init,"g_scene_textures…–Œ¥≥ı ºªØ£°"); }
		void create();

		vk_device* device;
		vk_swapchain* swapchain;
	};

	extern scene_textures g_scene_textures;
}}