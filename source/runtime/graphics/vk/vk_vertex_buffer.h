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

		inline void bind_and_draw(VkCommandBuffer cmd_buf)
		{
			vkCmdBindIndexBuffer(cmd_buf, *buffer, 0, index_type);
			vkCmdDrawIndexed(cmd_buf, index_count, 1, 0, 0, 0);
		}

		static std::shared_ptr<vk_index_buffer> create(vk_device* vulkanDevice,VkCommandPool pool,const std::vector<uint16_t>& indices);

		static std::shared_ptr<vk_index_buffer> create(vk_device* vulkanDevice,VkCommandPool pool,const std::vector<uint32_t>& indices);

	private:
		vk_device* device;

	public:
		std::shared_ptr<vk_buffer> buffer = nullptr;
		int32_t	index_count = 0;
		VkIndexType	index_type = VK_INDEX_TYPE_UINT32;
	};

	enum class vertex_attribute
	{
		none = 0,

		pos,	// vec3

		uv0,    // vec2
		uv1,	// vec2

		normal, // vec3
		tangent,// vec4
		color,  // vec3

		alpha,  // float

		skin_weight, // vec4
		skin_index,  // vec4
		skin_pack,   // vec3
		
		instance_float, // float
		instance_vec2,  // vec2
		instance_vec3,  // vec3
		instance_vec4,  // vec4

		uv2,	// vec4
		uv3,	// vec4
		uv4,	// vec4
		uv5,	// vec4
		uv6,	// vec4

		count,
	};

	inline int32_t vertex_attribute_size(vertex_attribute va)
	{
		switch(va)
		{
		case flower::graphics::vertex_attribute::instance_float:
		case flower::graphics::vertex_attribute::alpha:
			return sizeof(float);
			break;

		case flower::graphics::vertex_attribute::uv0:
		case flower::graphics::vertex_attribute::uv1:
		case flower::graphics::vertex_attribute::instance_vec2:
			return 2 * sizeof(float);
			break;

		case flower::graphics::vertex_attribute::pos:
		case flower::graphics::vertex_attribute::color:
		case flower::graphics::vertex_attribute::normal:
		case flower::graphics::vertex_attribute::skin_pack:
		case flower::graphics::vertex_attribute::instance_vec3:
			return 3 * sizeof(float);
			break;

		case flower::graphics::vertex_attribute::tangent:
		case flower::graphics::vertex_attribute::skin_weight:
		case flower::graphics::vertex_attribute::skin_index:
		case flower::graphics::vertex_attribute::instance_vec4:
		case flower::graphics::vertex_attribute::uv2:
		case flower::graphics::vertex_attribute::uv3:
		case flower::graphics::vertex_attribute::uv4:
		case flower::graphics::vertex_attribute::uv5:
		case flower::graphics::vertex_attribute::uv6:
			return 4 * sizeof(float);
			break;

		case flower::graphics::vertex_attribute::none:
		case flower::graphics::vertex_attribute::count:
		default:
			LOG_VULKAN_ERROR("使用未定义的vertex_attribute类型！");
			return 0;
			break;
		}

		return 0;
	}

	inline int32_t vertex_attribute_count(vertex_attribute va)
	{
		return vertex_attribute_size(va) / sizeof(float);
	}

	inline VkFormat VertexAttributeToVkFormat(vertex_attribute va)
	{
		VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
		
		switch(va)
		{
		case flower::graphics::vertex_attribute::alpha:
		case flower::graphics::vertex_attribute::instance_float:
			format = VK_FORMAT_R32_SFLOAT;
			break;

		case flower::graphics::vertex_attribute::uv0:
		case flower::graphics::vertex_attribute::uv1:
		case flower::graphics::vertex_attribute::instance_vec2:
			format = VK_FORMAT_R32G32_SFLOAT;
			break;

		case flower::graphics::vertex_attribute::pos:
		case flower::graphics::vertex_attribute::normal:
		case flower::graphics::vertex_attribute::color:
		case flower::graphics::vertex_attribute::skin_pack:
		case flower::graphics::vertex_attribute::instance_vec3:
			format = VK_FORMAT_R32G32B32_SFLOAT;
			break;

		case flower::graphics::vertex_attribute::tangent:
		case flower::graphics::vertex_attribute::skin_weight:
		case flower::graphics::vertex_attribute::skin_index:
		case flower::graphics::vertex_attribute::uv2:
		case flower::graphics::vertex_attribute::uv3:
		case flower::graphics::vertex_attribute::uv4:
		case flower::graphics::vertex_attribute::uv5:
		case flower::graphics::vertex_attribute::uv6:
		case flower::graphics::vertex_attribute::instance_vec4:
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;

		case flower::graphics::vertex_attribute::none:
		case flower::graphics::vertex_attribute::count:
		default:
			LOG_VULKAN_ERROR("使用未定义的vertex_attribute类型！");
			break;
		}

		return format;
	}

	class vk_vertex_buffer
	{
	public:
		vk_vertex_buffer(vk_device* in_device) : device(in_device) { }
		~vk_vertex_buffer() { }

		inline void bind(VkCommandBuffer cmdBuffer)
		{
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(buffer->buffer), &offset);
		}

		VkVertexInputBindingDescription get_input_binding();

		std::vector<VkVertexInputAttributeDescription> get_input_attribute(const std::vector<vertex_attribute>& shader_inputs);

		static std::shared_ptr<vk_vertex_buffer> create(vk_device* in_device,VkCommandPool pool,const std::vector<float>& vertices,const std::vector<vertex_attribute>& attributes);

		std::shared_ptr<vk_buffer> buffer;
		VkDeviceSize offset = 0;
		std::vector<vertex_attribute> attributes;

	private:
		vk_device* device;
	};
} }