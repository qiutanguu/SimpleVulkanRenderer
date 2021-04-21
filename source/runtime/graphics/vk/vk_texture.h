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

		static sampler_layout linear_repeat()
		{
			sampler_layout ret {};
			ret.mag_filter = VK_FILTER_LINEAR;
			ret.min_filter = VK_FILTER_LINEAR;

			ret.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			ret.address_mode_U = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
			ret.address_mode_V = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
			ret.address_mode_W = VK_SAMPLER_ADDRESS_MODE_REPEAT;

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

		static std::shared_ptr<vk_texture> create_depth_no_msaa(
			vk_device* in_device,
			vk_swapchain* in_swapchain
		);

		static std::shared_ptr<vk_texture> create_rt(
			vk_device* in_device,
			VkFormat format,
			vk_swapchain* in_swapchain,
			VkImageAspectFlags aspect, 
			VkImageUsageFlags usage
		);
	};

} }