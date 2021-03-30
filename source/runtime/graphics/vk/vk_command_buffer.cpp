#include "vk_command_buffer.h"

namespace flower { namespace graphics{

	vk_command_buffer::~vk_command_buffer()
	{
		if(command_buffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(device,pool,1,&command_buffer);
			command_buffer = VK_NULL_HANDLE;
		}

		if(fence!=VK_NULL_HANDLE)
		{
			vkDestroyFence(device,fence,nullptr);
			fence = VK_NULL_HANDLE;
		}
	}

	void vk_command_buffer::begin(VkCommandBufferUsageFlagBits flag)
	{
		if(is_begin)
		{
			return;
		}
		is_begin = true;

		VkCommandBufferBeginInfo command_buffer_begin_info{ };
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = flag;

		if(vkBeginCommandBuffer(command_buffer,&command_buffer_begin_info) != VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("ø™ ºº«¬ºªÊ÷∆√¸¡Óª∫≥Â ß∞‹£°");
		}
	}

	void vk_command_buffer::end()
	{
		if(!is_begin)
		{
			return;
		}

		is_begin = false;
		if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("º«¬ºªÊ÷∆√¸¡Óª∫≥ÂΩ· ¯ ß∞‹£°");
		}
	}

	void vk_command_buffer::submit(VkSemaphore signal_semaphore)
	{
		end();

		VkSubmitInfo submit_info{ };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		if(signal_semaphore != VK_NULL_HANDLE)
		{
			submit_info.signalSemaphoreCount =  1;
			submit_info.pSignalSemaphores = &signal_semaphore;
		}
		else
		{
			submit_info.signalSemaphoreCount = 0;
		}

		if(wait_flags.size()>0)
		{
			submit_info.waitSemaphoreCount = wait_semaphores.size();
			submit_info.pWaitSemaphores = wait_semaphores.data();
			submit_info.pWaitDstStageMask = wait_flags.data();
		}

		vkResetFences(device,1,&fence);
		if(vkQueueSubmit(queue,1,&submit_info,fence) != VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("Ã·ΩªªÊ÷∆command buffer ß∞‹!");
		}
		vkWaitForFences(device,1,&fence,true,UINT64_MAX);
	}

	std::shared_ptr<vk_command_buffer> vk_command_buffer::create(
		vk_device& in_device,
		VkCommandPool command_pool,
		VkCommandBufferLevel level,
		VkQueue queue)
	{
		auto ret_command_buffer = std::make_shared<vk_command_buffer>(in_device); 
		ret_command_buffer->pool = command_pool;
		ret_command_buffer->is_begin = false;

		if(queue != VK_NULL_HANDLE)
		{
			ret_command_buffer->queue = queue;
		}
		else
		{
			ret_command_buffer->queue = in_device.graphics_queue;
		}

		VkCommandBufferAllocateInfo cmdBufferAllocateInfo {};
		cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocateInfo.commandPool = ret_command_buffer->pool;
		cmdBufferAllocateInfo.level = level;
		cmdBufferAllocateInfo.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(ret_command_buffer->device,&cmdBufferAllocateInfo,&(ret_command_buffer->command_buffer)) != VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("…Í«ÎÕº–ŒCommandBuffer ß∞‹£°");
		}

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		vkCreateFence(ret_command_buffer->device,&fenceCreateInfo,nullptr,&(ret_command_buffer->fence));

		return ret_command_buffer;
	}


} }

