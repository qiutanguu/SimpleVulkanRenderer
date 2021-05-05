#include "lighting_pass.h"
#include "../shader_manager.h"

namespace flower{namespace graphics{

	std::shared_ptr<lighting_pass> lighting_pass::create(
		vk_renderpass_mix_data in_mixdata
	)
	{
		auto ret = std::make_shared<lighting_pass>(in_mixdata);
		ret->create_renderpass();
		ret->create_framebuffers();

		ret->cmd_buf = vk_command_buffer::create(*in_mixdata.device,in_mixdata.pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkSemaphoreCreateInfo semaphore_create_info {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_check(vkCreateSemaphore(*in_mixdata.device, &semaphore_create_info, nullptr, &ret->lighting_pass_semaphore));

		ret->lighting_material = material_lighting::create(in_mixdata.device,ret->render_pass,in_mixdata.pool);

		return ret;
	}

	lighting_pass::~lighting_pass()
	{
		destroy_framebuffers();
		destroy_renderpass();
		vkDestroySemaphore(*mix_data.device, lighting_pass_semaphore, nullptr);
	}

	void lighting_pass::create_framebuffers()
	{
		auto& scenecolor_ref = g_scene_textures.scene_color;

		const auto& width = scenecolor_ref->width;
		const auto& height = scenecolor_ref->height;

		std::array<VkImageView,1> attachments;
		attachments[0] = scenecolor_ref->image_view;

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = render_pass;
		fbufCreateInfo.pAttachments = attachments.data();
		fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		fbufCreateInfo.width = width;
		fbufCreateInfo.height = height;
		fbufCreateInfo.layers = 1;
		vk_check(vkCreateFramebuffer(*mix_data.device, &fbufCreateInfo, nullptr, &framebuffer));
	}

	void lighting_pass::destroy_framebuffers()
	{
		vkDestroyFramebuffer(*mix_data.device, framebuffer, nullptr);
	}

	// 创建lighting的renderpass
	void lighting_pass::create_renderpass()
	{
		std::array<VkAttachmentDescription,1> attachmentDescs{};

		attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescs[0].format = g_scene_textures.scene_color->format;

		std::vector<VkAttachmentReference> colorReferences;
		colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		VkAttachmentReference depthReference = {};
		depthReference.attachment = VK_ATTACHMENT_UNUSED;
		depthReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		subpass.pDepthStencilAttachment = &depthReference;

		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(*mix_data.device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建renderPass失败！");
		}
	}

	void lighting_pass::destroy_renderpass()
	{
		vkDestroyRenderPass(*mix_data.device, render_pass, nullptr);
	}

	std::shared_ptr<material_lighting> graphics::material_lighting::create(
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool)
	{
		auto ret = std::make_shared<material_lighting>();

		auto& shader = g_shader_manager.lighting_shader;

		vk_pipeline_info pipe_info{};	
		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;
		pipe_info.color_attachment_count = 1;

		pipe_info.rasterization_state.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipe_info.rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipe_info.depth_stencil_state.depthWriteEnable = VK_FALSE;
		pipe_info.depth_stencil_state.depthTestEnable = VK_FALSE;

		ret->pipeline = vk_pipeline::create_by_shader(
			indevice,
			VK_NULL_HANDLE,
			pipe_info,
			shader,
			in_renderpass
		);

		// 设置model矩阵
		ret->descriptor_set = shader->allocate_descriptor_set();
		ret->shader = shader;

		ret->descriptor_set->set_buffer("ub_directional_light",g_uniform_buffers.ubo_directional_light);

		ret->descriptor_set->set_image(
			"gbuffer_position_worldspace",
			g_scene_textures.position_worldspace
		);

		ret->descriptor_set->set_image(
			"gbuffer_normal_worldspace",
			g_scene_textures.normal_worldspace
		);

		ret->descriptor_set->set_image(
			"gbuffer_basecolor",
			g_scene_textures.basecolor
		);

		ret->descriptor_set->set_buffer("ub_directional_light_vp",g_uniform_buffers.ubo_directlight_vps);

		ret->descriptor_set->set_image(
			"directional_light_shadowdepth",
			g_scene_textures.scene_shadowdepth
		);

		g_scene_textures.scene_shadowdepth->update_sampler(
			sampler_layout::nearset_clamp()
		);
		ret->descriptor_set->set_image(
			"directional_light_shadowdepth_tex",
			g_scene_textures.scene_shadowdepth
		);
		g_scene_textures.scene_shadowdepth->update_sampler(
			sampler_layout::shadow_depth_pcf()
		);
		
		return ret;
	}
}}
