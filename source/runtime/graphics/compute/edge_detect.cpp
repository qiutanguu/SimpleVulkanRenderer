#include "edge_detect.h"
#include "../vk/vk_device.h"
#include "../shader_manager.h"

namespace flower { namespace graphics{

	compute_edge_detect::~compute_edge_detect()
	{
		vkDestroyPipeline(*device, pipeline, nullptr);
		vkDestroySemaphore(*device, semaphore, nullptr);

		cmd_buf.reset();
		edge_detect_compute_target.reset();
		descriptor_set.reset();
	}

	std::shared_ptr<compute_edge_detect> compute_edge_detect::create(vk_device* in_device,VkCommandPool in_pool)
	{
		auto ret = std::make_shared<compute_edge_detect>(in_device,in_pool);

		ret->cmd_buf = vk_command_buffer::create(*in_device,in_pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY,in_device->compute_queue);

		// 需要swapchain重建的部分
		ret->edge_detect_compute_target = vk_texture::create_storage_image_2d(ret->device,ret->pool,VK_FORMAT_R8G8B8A8_UNORM,g_scene_textures.basecolor->width,g_scene_textures.basecolor->height);

		ret->edge_detect_compute_target->update_sampler(
			sampler_layout::linear_clamp()
		);

		// compute pipeline prepare
		auto& shader = g_shader_manager.comp_edge_detect;
		ret->descriptor_set = shader->allocate_descriptor_set();

		g_scene_textures.basecolor->descriptor_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		g_scene_textures.basecolor->image_layout = VK_IMAGE_LAYOUT_GENERAL;

		ret->descriptor_set->set_image("inputImage",g_scene_textures.basecolor);

		g_scene_textures.basecolor->descriptor_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		g_scene_textures.basecolor->image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		ret->descriptor_set->set_image("resultImage",ret->edge_detect_compute_target);


		ret->descriptor_set_layout_ref = shader->shader_descriptor_set_layouts;
		ret->pipeline_layout_ref = shader->pipeline_layout;

		// 创建compute pipeline
		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = ret->pipeline_layout_ref;
		computePipelineCreateInfo.flags = 0;
		computePipelineCreateInfo.stage = shader->comp_shader_module->stage_create_info;
		vk_check(vkCreateComputePipelines(*in_device, nullptr, 1, &computePipelineCreateInfo, nullptr, &ret->pipeline));

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_check(vkCreateSemaphore(*ret->device, &semaphoreCreateInfo, nullptr, &ret->semaphore));

		return ret;
	}

	void compute_edge_detect::build_command_buffer()
	{
		cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkImageMemoryBarrier imageBarrier {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.image = g_scene_textures.basecolor->image;
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.srcQueueFamilyIndex = device->find_queue_families().graphics_family;
		imageBarrier.dstQueueFamilyIndex = device->find_queue_families().compute_faimly;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; 

		vkCmdPipelineBarrier(
			*cmd_buf,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);

		vkCmdBindPipeline(*cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

		vkCmdBindDescriptorSets(*cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_ref, 0, 1, descriptor_set->descriptor_sets.data(), 0, 0);

		vkCmdDispatch(*cmd_buf, edge_detect_compute_target->width / 16, edge_detect_compute_target->height / 16, 1);

		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.srcQueueFamilyIndex = device->find_queue_families().compute_faimly;
		imageBarrier.dstQueueFamilyIndex = device->find_queue_families().graphics_family;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(
			*cmd_buf,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);

		cmd_buf->end();
	}

} }