#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/scene/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "core/camera.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/pass/gbuffer_pass.h"
#include "graphics/pass/texture_pass.h"
#include "graphics/scene/light.h"
#include "graphics/pass/lighting_pass.h"
#include "graphics/pass/tone_mapper_pass.h"
#include "graphics/pass/shadowdepth_pass.h"
#include "graphics/compute/edge_detect.h"

namespace flower{ namespace graphics{

	class pbr_deferred : public vk_runtime
	{
	public:
		pbr_deferred(GLFWwindow* window) : 
			vk_runtime::vk_runtime(window)
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
		void update_before_commit(uint32_t backBuffer_index);
		void record_renderCommand();

		std::shared_ptr<gbuffer_pass> pass_gbuffer;
		void gbuffer_record_command();

		std::shared_ptr<lighting_pass> pass_lighting;
		void lighting_record_command();

		std::shared_ptr<shadowdepth_pass> pass_shadowdepth;
		void shadowdepth_record_command();

		std::shared_ptr<compute_edge_detect> compute_edge_detect_pass;

		std::shared_ptr<tonemapper_pass> pass_tonemapper;
		std::shared_ptr<ui_overlay> ui_context;
		void tonemapper_ui_record_command();
		void ui_overlay_update();
		void ui_layout();

		bool start_play = false;
	};

} }