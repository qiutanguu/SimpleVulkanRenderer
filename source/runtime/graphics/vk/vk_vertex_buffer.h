#pragma once
#include "vulkan/vulkan.h"
#include "vk_buffer.h"
#include "vk_device.h"
#include "vk_command_buffer.h"

namespace flower { namespace graphics{

	class vk_index_buffer
	{
	public:
		vk_index_buffer(vk_device* in_device) : device(in_device) {  }
		~vk_index_buffer() {  }


		inline void bind(VkCommandBuffer cmd_buf)
		{
			vkCmdBindIndexBuffer(cmd_buf, *buffer, 0, index_type);
		}

		static std::shared_ptr<vk_index_buffer> create(vk_device* vulkanDevice,VkCommandPool pool,std::vector<uint16_t> indices);
		static std::shared_ptr<vk_index_buffer> create(vk_device* vulkanDevice,VkCommandPool pool,std::vector<uint32_t> indices);

	private:
		vk_device* device;

	public:
		std::shared_ptr<vk_buffer> buffer = nullptr;
		int32_t	index_count = 0;
		VkIndexType	index_type = VK_INDEX_TYPE_UINT32;
	};

	class vk_vertex_buffer
	{
	public:
		vk_vertex_buffer(vk_device* in_device) : device (in_device){ }
		~vk_vertex_buffer(){  }

		inline void bind(VkCommandBuffer cmdBuffer)
		{
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(buffer->buffer), &offset);
		}
		
		template<typename vertex_type>
		inline static std::shared_ptr<vk_vertex_buffer> create(vk_device* in_device,VkCommandPool pool,std::vector<vertex_type> vertices)
		{
			std::shared_ptr<vk_vertex_buffer> ret = std::make_shared<vk_vertex_buffer>(in_device);
			ret->get_binding_func = vertex_type::get_binding;
			ret->get_attributes_func = vertex_type::get_attributes;
			VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

			auto stageBuffer = vk_buffer::create(
				*in_device,
				pool,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				(void *)(vertices.data())
			);

			ret->buffer = vk_buffer::create(
				*in_device,
				pool,  
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				bufferSize,
				nullptr
			);

			ret->buffer->stage_copy_from(*stageBuffer, bufferSize,in_device->graphics_queue);
			return ret;
		}
	
	private:
		vk_device* device;
	public:
		std::shared_ptr<vk_buffer> buffer = nullptr;
		VkDeviceSize offset = 0;

		std::function<VkVertexInputBindingDescription()> get_binding_func;
		std::function<std::vector<VkVertexInputAttributeDescription>()> get_attributes_func;
	};

	
} }