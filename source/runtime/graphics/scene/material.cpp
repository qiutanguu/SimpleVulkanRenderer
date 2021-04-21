#include "material.h"
#include "../texture_manager.h"
#include "../shader_manager.h"

namespace flower { namespace graphics{

	// pbr 着色器的渲染管线创建
	void material_gbuffer::on_create(vk_device* indevice,VkRenderPass in_renderpass)
	{
		shader = g_shader_manager.gbuffer_shader;

		vk_pipeline_info pipe_info;

		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;

		pipeline = vk_pipeline::create_by_shader(
			indevice,
			VK_NULL_HANDLE,
			pipe_info,
			shader,
			in_renderpass
		);

		descriptor_set = shader->allocate_descriptor_set();

		// 设置描述符集参数
		descriptor_set->set_buffer("ub_vp",g_uniform_buffers.ubo_vps);
		descriptor_set->set_buffer("ub_m",model_ubo);
		descriptor_set->set_image("basecolor_tex",g_texture_manager.get_texture_vk(map_diffuse));
		descriptor_set->set_image("normal_tex",g_texture_manager.get_texture_vk(map_normal));
	}

	void material_gbuffer::on_swapchain_recreate(vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass)
	{
		pipeline.reset();
		on_create(indevice,in_renderpass);
	}

} }