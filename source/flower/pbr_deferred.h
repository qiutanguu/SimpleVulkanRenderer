#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "graphics/vk/vk_texture.h"
#include "core/camera.h"
#include "graphics/vk/vk_shader.h"

namespace flower{ namespace graphics{

	class pbr_deferred : public vk_runtime
	{
	public:
		pbr_deferred(camera& incam,GLFWwindow* window) : 
			vk_runtime::vk_runtime(window),
			scene_view_cam(incam)
		{

		}

	public:
		virtual void config_before_init() override;
		virtual void initialize_special() override;
		virtual void tick(float time, float delta_time) override;
		virtual void destroy_special() override;
		virtual void recreate_swapchain() override;
		virtual void cleanup_swapchain() override;

	private:
		std::vector<std::shared_ptr<vk_buffer>> buffer_ubo_vp;
		void create_uniform_buffer();

		void update_before_commit(uint32_t backBuffer_index);
		void record_renderCommand();

		mesh mesh_data = {};
		mesh mesh_sponza = {};

		// pipeline ��Ⱦ����
		std::shared_ptr<vk_pipeline> pipeline_render;
		void createGraphicsPipeline();

		std::shared_ptr<vk_shader_mix> texture_shader_mix;
		std::vector<std::shared_ptr<vk_descriptor_set>> texture_descriptor_sets;

		// ����
		std::shared_ptr<vk_texture> mesh_texture;
		std::vector<std::shared_ptr<vk_texture>> sponza_textures;
		void createTextureImage();

		// ����buffer
		std::shared_ptr<vk_vertex_buffer> sponza_vertex_buf;
		std::vector<std::shared_ptr<vk_index_buffer>> sponza_index_buffer;

		void upload_vertex_buffer();

		camera& scene_view_cam;
	};

} }