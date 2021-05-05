#include "shadowdepth_pass.h"
#include "../shader_manager.h"

namespace flower{namespace graphics{

	std::shared_ptr<shadowdepth_pass> shadowdepth_pass::create(
		vk_renderpass_mix_data in_mixdata
	)
	{
		auto ret = std::make_shared<shadowdepth_pass>(in_mixdata);
		ret->create_renderpass();
		ret->create_framebuffers();

		ret->cmd_buf = vk_command_buffer::create(*in_mixdata.device,in_mixdata.pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VkSemaphoreCreateInfo semaphore_create_info {};

		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_check(vkCreateSemaphore(*in_mixdata.device, &semaphore_create_info, nullptr, &ret->shadowdepth_semaphore));

		return ret;
	}

	shadowdepth_pass::~shadowdepth_pass()
	{
		destroy_framebuffers();
		destroy_renderpass();
		vkDestroySemaphore(*mix_data.device, shadowdepth_semaphore, nullptr);
	}

	void shadowdepth_pass::create_framebuffers()
	{
		auto& depthonly_ref = g_scene_textures.scene_shadowdepth;

		const auto& width = depthonly_ref->width;
		const auto& height = depthonly_ref->height;

		std::array<VkImageView,1> attachments;
		attachments[0] = depthonly_ref->image_view;

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

	void shadowdepth_pass::destroy_framebuffers()
	{
		vkDestroyFramebuffer(*mix_data.device, framebuffer, nullptr);
	}

	// ����Gbuffer��renderpass
	void shadowdepth_pass::create_renderpass()
	{
		std::array<VkAttachmentDescription,1> attachmentDescs{};

		attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescs[0].format = g_scene_textures.scene_shadowdepth->format;
	
		VkAttachmentReference colorReference = {};
		colorReference.attachment = VK_ATTACHMENT_UNUSED;
		colorReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pDepthStencilAttachment = &depthReference;

		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(*mix_data.device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("����renderPassʧ�ܣ�");
		}
	}

	void shadowdepth_pass::destroy_renderpass()
	{
		vkDestroyRenderPass(*mix_data.device, render_pass, nullptr);
	}

	std::shared_ptr<material_shadowdepth> material_shadowdepth::create(
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool,
		const std::vector<uint32_t>& in_texlib,
		glm::mat4 model_mat)
	{
		auto ret = std::make_shared<material_shadowdepth>();

		auto& shader = g_shader_manager.shadowdepth_shader;

		vk_pipeline_info pipe_info;	

		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;
		pipe_info.color_attachment_count = 0;
		
		// ShadowMap���ƫ�ƿ���
		pipe_info.rasterization_state.depthBiasEnable = VK_TRUE;

		// ShadowMap Cull Front
		pipe_info.rasterization_state.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipe_info.depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_DEPTH_BIAS
		};

		ret->pipeline = vk_pipeline::create_by_shader(
			indevice,
			VK_NULL_HANDLE,
			pipe_info,
			shader,
			in_renderpass,
			dynamicStateEnables
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

		ret->descriptor_set->set_buffer("ub_vp",g_uniform_buffers.ubo_directlight_vps);
		ret->descriptor_set->set_buffer("ub_m",ret->model_ubo);

		ret->descriptor_set->set_image(
			"mask_tex",
			g_texture_manager.get_texture_vk(in_texlib[texture_id_type::mask])
		);

		return ret;
	}
}}
