#pragma once
#include "vk_common.h"
#include "vk_command_buffer.h"

namespace flower { namespace graphics{

	struct vk_texture
	{
		VkDevice device;
		VkImage image;
		VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDeviceMemory image_memory = VK_NULL_HANDLE;
		VkImageView image_view = VK_NULL_HANDLE;
		VkSampler image_sampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo descriptor_info;
		int32_t width = 0;
		int32_t height = 0;
		int32_t depth = 1;
		int32_t mipLevels = 0;
		int32_t layerCount = 1;
		VkSampleCountFlagBits num_samples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		bool is_cube_map = false;

		vk_texture(){ }
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
			const uint8_t* rgbaData, 
			uint32_t size, 
			VkFormat format, 
			int32_t width, 
			int32_t height, 
			vk_device* in_device, 
			std::shared_ptr<vk_command_buffer> in_cmd_buffer, 
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
		); 
	#pragma endregion
	};

} }