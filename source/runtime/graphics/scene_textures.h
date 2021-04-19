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
			scenecolor.reset();
			gbuffer_a.reset();
			gbuffer_b.reset();
			gbuffer_c.reset();
			gbuffer_d.reset();
			gbuffer_e.reset();
			velocity_rt.reset();
			scene_depth_stencil.reset();
		}


	public:
		std::shared_ptr<vk_texture> scenecolor; // r16 g16 b16 a16

		std::shared_ptr<vk_texture> gbuffer_a;
		std::shared_ptr<vk_texture> gbuffer_b;
		std::shared_ptr<vk_texture> gbuffer_c;
		std::shared_ptr<vk_texture> gbuffer_d;
		std::shared_ptr<vk_texture> gbuffer_e;
		std::shared_ptr<vk_texture> velocity_rt;

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