#pragma once
#include "vk_common.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_swapchain.h"

namespace flower { namespace graphics{

	struct sampler_layout
	{
		VkFilter mag_filter;
		VkFilter min_filter;
		VkSamplerMipmapMode mipmap_mode;
		VkSamplerAddressMode address_mode_U; 
		VkSamplerAddressMode address_mode_V; 
		VkSamplerAddressMode address_mode_W;
		VkCompareOp compare_op;
		VkBool32 compareEnable;
		VkBorderColor bordercolor;

		static sampler_layout linear_repeat()
		{
			sampler_layout ret {};
			ret.mag_filter = VK_FILTER_LINEAR;
			ret.min_filter = VK_FILTER_LINEAR;

			ret.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			ret.address_mode_U = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
			ret.address_mode_V = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
			ret.address_mode_W = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			ret.compare_op = VK_COMPARE_OP_ALWAYS;
			ret.compareEnable = VK_FALSE;

			ret.bordercolor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			return ret;
		}

		static sampler_layout linear_clamp()
		{
			sampler_layout ret {};
			ret.mag_filter = VK_FILTER_LINEAR;
			ret.min_filter = VK_FILTER_LINEAR;
			ret.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			ret.address_mode_U = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; 
			ret.address_mode_V = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; 
			ret.address_mode_W = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			ret.bordercolor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			ret.compare_op = VK_COMPARE_OP_ALWAYS;
			ret.compareEnable = VK_FALSE;
			return ret;
		}

		static sampler_layout nearset_clamp()
		{
			sampler_layout ret {};
			ret.mag_filter = VK_FILTER_NEAREST;
			ret.min_filter = VK_FILTER_NEAREST;
			ret.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			ret.address_mode_U = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; 
			ret.address_mode_V = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; 
			ret.address_mode_W = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			ret.bordercolor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

			ret.compare_op = VK_COMPARE_OP_ALWAYS;
			ret.compareEnable = VK_FALSE;
			return ret;
		}

		static sampler_layout shadow_depth_pcf()
		{
			sampler_layout ret {};

			ret.mag_filter = VK_FILTER_LINEAR;
			ret.min_filter = VK_FILTER_LINEAR;
			ret.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			ret.address_mode_U = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; 
			ret.address_mode_V = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; 
			ret.address_mode_W = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			ret.bordercolor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;


			ret.compare_op = VK_COMPARE_OP_LESS;
			ret.compareEnable = VK_TRUE;
			return ret;
		}
	};

	class vk_texture
	{
	private:
		vk_device* device;

	public:
		uint32_t mip_levels = 0;
		
		VkImage image;
		VkDeviceMemory image_memory = VK_NULL_HANDLE;
		VkImageView image_view = VK_NULL_HANDLE;
		VkSampler image_sampler = VK_NULL_HANDLE;

		VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkDescriptorImageInfo descriptor_info;

		int32_t width = 0;
		int32_t height = 0;
		int32_t depth = 1; // depth

		// cubemap 为6,Texture Array 为自定义
		int32_t layer_count = 1;
		VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;

		bool is_cube_map = false;

		VkExtent2D get_extent_2d(){ VkExtent2D ret{}; ret.width = width;ret.height = height; return ret; }

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		vk_texture(vk_device* in_device): device(in_device){ }
		~vk_texture();
		void update_sampler(
			const sampler_layout& in
		);

		static std::shared_ptr<vk_texture> create_2d_mipmap(
			vk_device* in_device,
			VkCommandPool in_pool,
			VkFormat format,
			const std::string& image_path
		); 

		static std::shared_ptr<vk_texture> create_storage_image_2d(
			vk_device* in_device,
			VkCommandPool in_pool,
			VkFormat format,
			int32_t width,
			int32_t height
		);

		static std::shared_ptr<vk_texture> create_depth_no_msaa(
			vk_device* in_device,
			vk_swapchain* in_swapchain
		);

		static std::shared_ptr<vk_texture> create_depthonly_no_msaa(
			vk_device* in_device,
			vk_swapchain* in_swapchain,
			int32_t width = -1,
			int32_t height = -1
		);

		static std::shared_ptr<vk_texture> create_color_attachment(
			vk_device* in_device,
			VkFormat format,
			vk_swapchain* in_swapchain
		);
	};

} }