#include "vk_texture.h"

namespace flower { namespace graphics{
	
	vk_texture::~vk_texture()
	{
        if (image_view != VK_NULL_HANDLE) 
        {
            vkDestroyImageView(device, image_view, nullptr);
            image_view = VK_NULL_HANDLE;
        }

        if (image != VK_NULL_HANDLE) 
        {
            vkDestroyImage(device, image, nullptr);
            image = VK_NULL_HANDLE;
        }

        if (image_sampler != VK_NULL_HANDLE) 
        {
            vkDestroySampler(device, image_sampler, nullptr);
            image_sampler = VK_NULL_HANDLE;
        }

        if (image_memory != VK_NULL_HANDLE) 
        {
            vkFreeMemory(device, image_memory, nullptr);
            image_memory = VK_NULL_HANDLE;
        }
	}

    void vk_texture::update_sampler(
        VkFilter mag_filter,
        VkFilter min_filter,
        VkSamplerMipmapMode mipmap_mode,
        VkSamplerAddressMode address_mode_U,
        VkSamplerAddressMode address_mode_V,
        VkSamplerAddressMode address_mode_W
    )
    {
        VkSamplerCreateInfo sampler_info { };
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = mag_filter;
        sampler_info.minFilter  = min_filter;
        sampler_info.mipmapMode = mipmap_mode;
        sampler_info.addressModeU = address_mode_U;
        sampler_info.addressModeV = address_mode_V;
        sampler_info.addressModeW = address_mode_W;
        sampler_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        sampler_info.maxAnisotropy = 1.0;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxLod = 1.0f;
        vk_check(vkCreateSampler(device, &sampler_info, nullptr, &image_sampler));

        if (descriptor_info.sampler) 
        {
            vkDestroySampler(device, descriptor_info.sampler, nullptr);
        }
        descriptor_info.sampler = image_sampler;
    }



}}