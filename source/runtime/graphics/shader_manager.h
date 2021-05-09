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
			tonemapper_shader.reset();
			shadowdepth_shader.reset();
			ui_fragment_shader.reset();
			ui_vertex_shader.reset();
			comp_edge_detect.reset();
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
		// graphics
		std::shared_ptr<vk_shader_mix> texture_map_shader;
		std::shared_ptr<vk_shader_mix> gbuffer_shader;
		std::shared_ptr<vk_shader_mix> lighting_shader;
		std::shared_ptr<vk_shader_mix> tonemapper_shader;
		std::shared_ptr<vk_shader_mix> shadowdepth_shader;

		// comp
		std::shared_ptr<vk_shader_mix> comp_edge_detect;


		// ui
		std::shared_ptr<vk_shader_module> ui_vertex_shader;
		std::shared_ptr<vk_shader_module> ui_fragment_shader;

	private:
		bool has_init = false;
		void check_init();

		vk_device* device;
	};


	extern shader_manager g_shader_manager;
} }