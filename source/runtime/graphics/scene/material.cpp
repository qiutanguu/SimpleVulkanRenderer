#include "material.h"
#include "../texture_manager.h"

namespace flower { namespace graphics{

	// pbr 着色器的渲染管线创建
	void material::create_renderpipeline(std::shared_ptr<vk_shader_mix> shader,
		vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass)
	{
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

		const auto& swapchain_num = swapchain->get_imageViews().size();
		descriptor_sets.resize(swapchain_num);

		for(size_t i = 0; i < swapchain_num; i++)
		{
			descriptor_sets[i] = shader->allocate_descriptor_set();

			// 设置描述符集参数
			descriptor_sets[i]->set_buffer("ub_vp",g_uniform_buffers.ubo_vps[i]);
			descriptor_sets[i]->set_buffer("ub_m",model_ubo);
			descriptor_sets[i]->set_image("base_color_texture",g_texture_manager.get_texture_vk(map_diffuse));
		}
	}

	void material::recreate_swapchain(std::shared_ptr<vk_shader_mix> in_shader,
		vk_device* indevice,vk_swapchain* swapchain,VkRenderPass in_renderpass)
	{
		pipeline.reset();
		descriptor_sets.resize(0);
		create_renderpipeline(in_shader,indevice,swapchain,in_renderpass);
	}

} }