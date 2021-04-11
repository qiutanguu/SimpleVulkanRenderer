#include "vk_command_buffer.h"

namespace flower { namespace graphics{

	vk_command_buffer::~vk_command_buffer()
	{
		if(command_buffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(device,pool,1,&command_buffer);
			command_buffer = VK_NULL_HANDLE;
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

	void vk_command_buffer::begin_onetime()
	{
		begin();
	}

	void vk_command_buffer::flush()
	{
		end();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(queue);
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

		return ret_command_buffer;
	}

} }

