#pragma once
#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk_device.h"

namespace flower { namespace graphics {
	
	class vk_buffer
	{
		friend class std::shared_ptr<vk_buffer>;
	private:
		void destroy();

	public:
		~vk_buffer() { destroy(); }

		// 废弃。请使用vk_buffer::create创建vk_buffer对象
		vk_buffer(vk_device& indevice) : device(indevice){ }

		static std::shared_ptr<vk_buffer> create(
			vk_device& in_device,
			VkCommandPool in_commandpool,
			VkBufferUsageFlags usageFlags, 
			VkMemoryPropertyFlags memoryPropertyFlags, 
			VkDeviceSize size, 
			void *data);
		
		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void setup_descriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void copy_to(void* data, VkDeviceSize size);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void stage_copy_from(vk_buffer& inBuffer,VkDeviceSize size,VkQueue execute_queue);

		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void* mapped = nullptr;

		VkDescriptorBufferInfo descriptor;
		VkDeviceSize size = 0;
		VkDeviceSize alignment = 0;
		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;

	private:
		bool create_buffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data);

		vk_device& device;
		VkCommandPool commandpool;
	};
}}