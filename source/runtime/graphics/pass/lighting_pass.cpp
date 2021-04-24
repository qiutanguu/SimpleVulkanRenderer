#include "lighting_pass.h"
#include "../shader_manager.h"

namespace flower{namespace graphics{

	std::shared_ptr<lighting_pass> lighting_pass::create(
		vk_renderpass_mix_data in_mixdata,
		VkCommandPool pool
	)
	{
		auto ret = std::make_shared<lighting_pass>(in_mixdata);
		ret->create_renderpass();
		ret->create_framebuffers();

		ret->cmd_buf = vk_command_buffer::create(*in_mixdata.device,pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		ret->pool = pool;
		VkSemaphoreCreateInfo semaphore_create_info {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_check(vkCreateSemaphore(*in_mixdata.device, &semaphore_create_info, nullptr, &ret->gbuffer_semaphore));

		return ret;
	}

	lighting_pass::~lighting_pass()
	{
		destroy_framebuffers();
		destroy_renderpass();
		vkDestroySemaphore(*mix_data.device, gbuffer_semaphore, nullptr);
	}

	void lighting_pass::create_framebuffers()
	{
		auto& position_ref = g_scene_textures.position_worldspace;
		auto& normal_ref = g_scene_textures.normal_worldspace;
		auto& basecolor_ref = g_scene_textures.basecolor;

		auto& scenecolor_ref = g_scene_textures.scene_color;

		const auto& width = basecolor_ref->width;
		const auto& height = basecolor_ref->height;

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
		depthReference.attachment = 0;
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
		VkCommandPool in_pool,
		const std::vector<uint32_t>& in_texlib,
		glm::mat4 model_mat)
	{
		auto ret = std::make_shared<material_lighting>();

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

		// 设置model矩阵
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

		return ret;
	}
}}
