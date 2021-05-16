#include "gbuffer_pass.h"
#include "../shader_manager.h"

namespace flower{namespace graphics{
	
	std::shared_ptr<gbuffer_pass> gbuffer_pass::create(
		vk_renderpass_mix_data in_mixdata
	)
	{
		auto ret = std::make_shared<gbuffer_pass>(in_mixdata);
		ret->create_renderpass();
		ret->create_framebuffers();

		ret->cmd_buf = vk_command_buffer::create(*in_mixdata.device,in_mixdata.pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkSemaphoreCreateInfo semaphore_create_info {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_check(vkCreateSemaphore(*in_mixdata.device, &semaphore_create_info, nullptr, &ret->gbuffer_semaphore));

		return ret;
	}

	gbuffer_pass::~gbuffer_pass()
	{
		destroy_framebuffers();
		destroy_renderpass();
		vkDestroySemaphore(*mix_data.device, gbuffer_semaphore, nullptr);
	}

	void gbuffer_pass::create_framebuffers()
	{
		auto& position_ref = g_scene_textures.position_worldspace;
		auto& normal_ref = g_scene_textures.normal_worldspace;
		auto& basecolor_ref = g_scene_textures.basecolor;
		auto& depthstencil_ref = g_scene_textures.scene_depth_stencil;

		const auto& width = depthstencil_ref->width;
		const auto& height = depthstencil_ref->height;

		std::array<VkImageView,4> attachments;
		attachments[0] = position_ref->image_view;
		attachments[1] = normal_ref->image_view;
		attachments[2] = basecolor_ref->image_view;
		attachments[3] = depthstencil_ref->image_view;

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

	void gbuffer_pass::destroy_framebuffers()
	{
		vkDestroyFramebuffer(*mix_data.device, framebuffer, nullptr);
	}

	// ����Gbuffer��renderpass
	void gbuffer_pass::create_renderpass()
	{
		std::array<VkAttachmentDescription,4> attachmentDescs{};

		// ��ʼ�����ʵ�
		for (uint32_t i = 0; i < 4; ++i)
		{
			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			// ������stencil
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			if (i == 3)
			{
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
		}

		// Formats
		attachmentDescs[0].format = g_scene_textures.position_worldspace->format;
		attachmentDescs[1].format = g_scene_textures.normal_worldspace->format;
		attachmentDescs[2].format = g_scene_textures.basecolor->format;
		attachmentDescs[3].format = g_scene_textures.scene_depth_stencil->format;

		std::vector<VkAttachmentReference> colorReferences;
		colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		VkAttachmentReference depthReference = {};
		depthReference.attachment = 3;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
			LOG_VULKAN_FATAL("����renderPassʧ�ܣ�");
		}
	}

	void gbuffer_pass::destroy_renderpass()
	{
		vkDestroyRenderPass(*mix_data.device, render_pass, nullptr);
	}

	std::shared_ptr<material_gbuffer> graphics::material_gbuffer::create(
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool,
		const std::vector<uint32_t>& in_texlib,
		glm::mat4 model_mat)
	{
		auto ret = std::make_shared<material_gbuffer>();

		auto& shader = g_shader_manager.gbuffer_shader;

		vk_pipeline_info pipe_info;	

		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;
		pipe_info.color_attachment_count = 3;

		ret->pipeline = vk_pipeline::create_by_shader(
			indevice,
			VK_NULL_HANDLE,
			pipe_info,
			shader,
			in_renderpass
		);

		VkDeviceSize bufferSize = sizeof(glm::mat4);
		auto buffer = vk_buffer::create(
			*indevice,
			in_pool,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			nullptr
		);

		buffer->map();
		buffer->copy_to((void*)&model_mat,sizeof(model_mat));
		buffer->unmap();

		// ����model����
		ret->model_ubo = buffer;

		ret->descriptor_set = shader->allocate_descriptor_set();
		ret->shader = shader;

		ret->descriptor_set->set_buffer("ub_vp",g_uniform_buffers.ubo_vps);
		ret->descriptor_set->set_buffer("ub_m",ret->model_ubo);

		ret->descriptor_set->set_image(
			"basecolor_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::diffuse])
		);

		ret->descriptor_set->set_image(
			"normal_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::normal])
		);

		ret->descriptor_set->set_image(
			"metalic_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::metallic])
		);

		ret->descriptor_set->set_image(
			"roughness_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::roughness])
		);

		ret->descriptor_set->set_image(
			"mask_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::mask])
		);

		return ret;
	}

	std::shared_ptr<material_gbuffer_character> graphics::material_gbuffer_character::create(
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool,
		const std::vector<uint32_t>& in_texlib,
		glm::mat4 model_mat)
	{
		auto ret = std::make_shared<material_gbuffer_character>();

		auto& shader = g_shader_manager.gbuffer_character_shader;

		vk_pipeline_info pipe_info;	

		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;
		pipe_info.color_attachment_count = 3;

		ret->pipeline = vk_pipeline::create_by_shader(
			indevice,
			VK_NULL_HANDLE,
			pipe_info,
			shader,
			in_renderpass
		);

		VkDeviceSize bufferSize = sizeof(glm::mat4);
		auto buffer = vk_buffer::create(
			*indevice,
			in_pool,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			nullptr
		);


		buffer->map();
		buffer->copy_to((void*)&model_mat,sizeof(model_mat));
		buffer->unmap();

		// ����model����
		ret->model_ubo = buffer;

		ret->descriptor_set = shader->allocate_descriptor_set();
		ret->shader = shader;

		ret->descriptor_set->set_buffer("ub_vp",g_uniform_buffers.ubo_vps);
		ret->descriptor_set->set_buffer("ub_m",ret->model_ubo);

		ret->descriptor_set->set_image(
			"basecolor_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::diffuse])
		);

		return ret;
	}
}}
