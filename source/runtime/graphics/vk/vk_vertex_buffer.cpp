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

	VkVertexInputBindingDescription vk_vertex_buffer::get_input_binding()
	{
		int32_t stride = 0;

		for (int32_t i = 0; i < attributes.size(); i++) 
		{
			stride += vertex_attribute_size(attributes[i]);
		}

		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = stride;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBinding;
	}

	std::vector<VkVertexInputAttributeDescription> vk_vertex_buffer::get_input_attribute(const std::vector<vertex_attribute>& shader_inputs)
	{
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
		int32_t offset = 0;
		for (int32_t i = 0; i < shader_inputs.size(); i++)
		{
			VkVertexInputAttributeDescription inputAttribute = {};
			inputAttribute.binding  = 0;
			inputAttribute.location = i;
			inputAttribute.format = VertexAttributeToVkFormat(shader_inputs[i]);
			inputAttribute.offset = offset;
			offset += vertex_attribute_size(shader_inputs[i]);
			vertexInputAttributs.push_back(inputAttribute);
		}
		return vertexInputAttributs;
	}

	std::shared_ptr<vk_vertex_buffer> vk_vertex_buffer::create(vk_device* in_device,VkCommandPool pool,std::vector<float> vertices,const std::vector<vertex_attribute>& attributes)
	{
		auto ret = std::make_shared<vk_vertex_buffer>(in_device);
		ret->attributes = attributes;

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


}}