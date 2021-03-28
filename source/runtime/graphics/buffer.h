#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "device.h"

namespace flower { namespace graphics {
	
	class buffer
	{
	public:
		buffer(){  }
		~buffer() {}

		void initialize(device in_device,VkCommandPool in_commandpool,VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
		{
			if(HasCreated)
			{
				return;
			}

			vk_device = in_device;
			commandpool = in_commandpool;

			CreateBuffer(usageFlags, memoryPropertyFlags, size,data);
		}

		void destroy();

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void copyTo(void* data, VkDeviceSize size);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void StageCopyFrom(buffer& inBuffer,VkDeviceSize size,VkQueue execute_queue);

		bool HasCreated = false;
		VkBuffer vk_buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void* mapped = nullptr;

		VkDescriptorBufferInfo descriptor;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;
	private:
		bool CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data);

		device vk_device;
		VkCommandPool commandpool;
	};
}}