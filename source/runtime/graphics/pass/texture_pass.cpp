#include "texture_pass.h"
#include "../shader_manager.h"

namespace flower{ namespace graphics{

	std::shared_ptr<texture_pass> texture_pass::create(vk_renderpass_mix_data in_mixdata)
	{
		auto ret = std::make_shared<texture_pass>(in_mixdata);
		ret->create_renderpass();
		ret->create_framebuffers();
		return ret;
	}

	texture_pass::~texture_pass()
	{
		destroy_framebuffers();
		destroy_renderpass();
	}

	void texture_pass::create_framebuffers()
	{
		const auto& swap_num = mix_data.swapchain->get_imageViews().size();
		const auto& extent = mix_data.swapchain->get_swapchain_extent();

		swapchain_framebuffers.resize(swap_num);

		for (size_t i = 0; i < swap_num; i++)
		{
			std::array<VkImageView, 2> attachments = 
			{
				mix_data.swapchain->get_imageViews()[i],
				g_scene_textures.scene_depth_stencil->image_view
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

			// 在这里绑定对应的 render pass
			framebufferInfo.renderPass = render_pass; 

			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*mix_data.device, &framebufferInfo, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS) 
			{
				LOG_VULKAN_FATAL("创建VulkanFrameBuffer失败！");
			}
		}
	}

	void texture_pass::destroy_framebuffers()
	{
		for (auto& swapchain_framebuffer : swapchain_framebuffers) 
		{
			vkDestroyFramebuffer(*mix_data.device, swapchain_framebuffer, nullptr);
		}
	}

	void texture_pass::create_renderpass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = mix_data.swapchain->get_swapchain_image_format();

		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = find_depth_format(mix_data.device->physical_device);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(*mix_data.device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建renderPass失败！");
		}
	}

	void texture_pass::destroy_renderpass()
	{
		vkDestroyRenderPass(*mix_data.device, render_pass, nullptr);
	}

	std::shared_ptr<material_texture> material_texture::create(
		vk_device* indevice,
		VkRenderPass in_renderpass,
		VkCommandPool in_pool,
		uint32_t map_colortex,
		glm::mat4 model_mat
	)
	{
		auto ret = std::make_shared<material_texture>();

		auto& shader = g_shader_manager.texture_map_shader;

		vk_pipeline_info pipe_info;

		pipe_info.vert_shader_module = shader->vert_shader_module->handle;
		pipe_info.frag_shader_module = shader->frag_shader_module->handle;

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
		ret->shader = g_shader_manager.texture_map_shader;

		ret->descriptor_set->set_buffer("ub_vp",g_uniform_buffers.ubo_vps);
		ret->descriptor_set->set_buffer("ub_m",ret->model_ubo);
		ret->descriptor_set->set_image(
			"base_color_texture",
			g_texture_manager.get_texture_vk(map_colortex)
		);

		return ret;
	}

}}
