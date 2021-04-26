#pragma once
#include "vk/vk_shader.h"

namespace flower{ namespace graphics{

	class shader_manager
	{
	public:
		shader_manager(){ }
		~shader_manager() { }

		void release()
		{
			texture_map_shader.reset();
			gbuffer_shader.reset();
			lighting_shader.reset();
		}

		void initialize(vk_device* in_device) 
		{
			device = in_device; 
			has_init = true;
			create();
		}

	private:
		void create();

	public:
		std::shared_ptr<vk_shader_mix> texture_map_shader;
		std::shared_ptr<vk_shader_mix> gbuffer_shader;
		std::shared_ptr<vk_shader_mix> lighting_shader;

	private:
		bool has_init = false;
		void check_init();

		vk_device* device;
	};


	extern shader_manager g_shader_manager;
} }