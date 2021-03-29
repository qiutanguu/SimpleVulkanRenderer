#include "vk_buffer.h"

namespace flower{ namespace graphics{

	void vk_buffer::create(vk_device in_device,VkCommandPool in_commandpool,VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
	{
		if(has_created)
		{
			LOG_VULKAN_ERROR("VkBuffer已经创建！再次创建将引起内存泄漏！");
			return;
		}

		device = in_device;
		commandpool = in_commandpool;

		create_buffer(usageFlags, memoryPropertyFlags, size,data);
	}

	VkResult vk_buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	void vk_buffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}

	VkResult vk_buffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(device, buffer, memory, offset);
	}

	void vk_buffer::setup_descriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	void vk_buffer::copy_to(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	VkResult vk_buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}

	VkResult vk_buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

	bool vk_buffer::create_buffer(VkBufferUsageFlags usageFlags,VkMemoryPropertyFlags memoryPropertyFlags,VkDeviceSize size,void* data) 
	{
		if(has_created)
		{
			return false;
		}

		//1. 创建 buffer 句柄
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usageFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建vulkan vk_buffer失败！");
		}

		// 2. 分配内存
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.find_memory_type(memRequirements.memoryTypeBits, memoryPropertyFlags);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("申请vulkan vk_buffer内存失败!");
		}

		if (data != nullptr)
		{
			void *mapped;
			vkMapMemory(device, memory, 0, size, 0, &mapped); // 内存映射
			memcpy(mapped, data, size); // 内存复制
			vkUnmapMemory(device,memory);// 内存取消映射
		}

		// 内存绑定到句柄中
		vkBindBufferMemory(device, buffer, memory, 0);

		has_created = true;
		return true;
	}


	void vk_buffer::stage_copy_from(vk_buffer& inBuffer,VkDeviceSize size,VkQueue execute_queue)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandpool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};

		// 偏移，目前设为0
		copyRegion.srcOffset = 0; 
		copyRegion.dstOffset = 0; 
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, inBuffer.buffer, this->buffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(execute_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(execute_queue);
		vkFreeCommandBuffers(device, commandpool, 1, &commandBuffer);
	}

	void vk_buffer::destroy()
	{
		if(!has_created)
		{
			LOG_VULKAN_ERROR("正在析构未初始化的Buffer！");
			return;
		}

		if (buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, buffer, nullptr);
		}
		if (memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(device, memory, nullptr);
		}
		has_created = false;
	}
} }