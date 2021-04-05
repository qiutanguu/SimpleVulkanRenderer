#include "vk_vertex_buffer.h"

namespace flower { namespace graphics{
	
	std::shared_ptr<vk_index_buffer> vk_index_buffer::create(vk_device* in_device,VkCommandPool pool,std::vector<uint32_t> indices)
	{
		std::shared_ptr<vk_index_buffer> ret = std::make_shared<vk_index_buffer>(in_device);
		ret->index_count = (int32_t) indices.size();
		ret->index_type = VK_INDEX_TYPE_UINT32;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		auto stageBuffer = vk_buffer::create(
			*in_device,
			pool,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			(void *)(indices.data())
		);

		ret->buffer = vk_buffer::create(
			*in_device,
			pool,  
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			bufferSize,
			nullptr
		);

		ret->buffer->stage_copy_from(*stageBuffer, bufferSize,in_device->graphics_queue);
		return ret;
	}


	std::shared_ptr<vk_index_buffer> vk_index_buffer::create(vk_device* in_device,VkCommandPool pool,std::vector<uint16_t> indices)
	{
		std::shared_ptr<vk_index_buffer> ret = std::make_shared<vk_index_buffer>(in_device);
		ret->index_count = (int32_t)indices.size();
		ret->index_type = VK_INDEX_TYPE_UINT16;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		auto stageBuffer = vk_buffer::create(
			*in_device,
			pool,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			(void *)(indices.data())
		);

		ret->buffer = vk_buffer::create(
			*in_device,
			pool,  
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			bufferSize,
			nullptr
		);

		ret->buffer->stage_copy_from(*stageBuffer, bufferSize,in_device->graphics_queue);
		return ret;
	}
} }