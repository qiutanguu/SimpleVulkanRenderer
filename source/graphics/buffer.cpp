#include "buffer.h"

namespace flower{ namespace graphics{

	VkResult buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(vk_device.logic_device, memory, offset, size, 0, &mapped);
	}

	void buffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(vk_device.logic_device, memory);
			mapped = nullptr;
		}
	}

	VkResult buffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(vk_device.logic_device, vk_buffer, memory, offset);
	}

	void buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = vk_buffer;
		descriptor.range = size;
	}

	void buffer::copyTo(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	VkResult buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(vk_device.logic_device, 1, &mappedRange);
	}

	VkResult buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(vk_device.logic_device, 1, &mappedRange);
	}

	bool buffer::CreateBuffer(VkBufferUsageFlags usageFlags,VkMemoryPropertyFlags memoryPropertyFlags,VkDeviceSize size,void* data) 
	{
		if(HasCreated)
		{
			return false;
		}

		//1. 创建 Buffer 句柄
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usageFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(vk_device.logic_device, &bufferInfo, nullptr, &vk_buffer) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建vulkan buffer失败！");
		}

		// 2. 分配内存
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vk_device.logic_device, vk_buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vk_device.find_memory_type(memRequirements.memoryTypeBits, memoryPropertyFlags);

		if (vkAllocateMemory(vk_device.logic_device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("申请vulkan buffer内存失败!");
		}

		if (data != nullptr)
		{
			void *mapped;
			vkMapMemory(vk_device.logic_device, memory, 0, size, 0, &mapped); // 内存映射
			memcpy(mapped, data, size); // 内存复制
			vkUnmapMemory(vk_device.logic_device,memory);// 内存取消映射
		}

		// 内存绑定到句柄中
		vkBindBufferMemory(vk_device.logic_device, vk_buffer, memory, 0);

		HasCreated = true;
		return true;
	}


	void buffer::StageCopyFrom(buffer& inBuffer,VkDeviceSize size,VkQueue execute_queue)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandpool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(vk_device.logic_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};

		// 偏移，目前设为0
		copyRegion.srcOffset = 0; 
		copyRegion.dstOffset = 0; 
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, inBuffer.vk_buffer, this->vk_buffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(execute_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(execute_queue);
		vkFreeCommandBuffers(vk_device.logic_device, commandpool, 1, &commandBuffer);
	}

	void buffer::destroy()
	{
		if(!HasCreated)
		{
			return;
		}

		if (vk_buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vk_device.logic_device, vk_buffer, nullptr);
		}
		if (memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(vk_device.logic_device, memory, nullptr);
		}
		HasCreated = false;
	}
} }