#pragma once
#include "vk_common.h"
#include "vk_command_buffer.h"
#include "vk_device.h"

namespace flower { namespace graphics{

	class vk_texture
	{
	private:
		vk_device* device;

	public:
		uint32_t mip_levels;
		
		VkImage image;
		VkDeviceMemory image_memory = VK_NULL_HANDLE;
		VkImageView image_view = VK_NULL_HANDLE;
		VkSampler image_sampler = VK_NULL_HANDLE;

		VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkDescriptorImageInfo descriptor_info{ };

		int32_t width = 0;
		int32_t height = 0;
		int32_t channels = 1;

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		vk_texture(vk_device* in_device): device(in_device){ }
		~vk_texture();
		void update_sampler(
			VkFilter mag_filter = VK_FILTER_LINEAR, 
			VkFilter min_filter = VK_FILTER_LINEAR,
			VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			VkSamplerAddressMode address_mode_U = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
			VkSamplerAddressMode address_mode_V = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
			VkSamplerAddressMode address_mode_W = VK_SAMPLER_ADDRESS_MODE_REPEAT
		);

	#pragma region creator
		static std::shared_ptr<vk_texture> create_2d(
			vk_device* in_device,
			VkCommandPool in_pool,
			const std::string& image_path
		); 
	#pragma endregion
	};

} }