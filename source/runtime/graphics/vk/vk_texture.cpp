#include "vk_texture.h"
#include "vk_buffer.h"

namespace flower { namespace graphics{

   
	
	vk_texture::~vk_texture()
	{
        if (image_view != VK_NULL_HANDLE) 
        {
            vkDestroyImageView(*device, image_view, nullptr);
            image_view = VK_NULL_HANDLE;
        }

        if (image != VK_NULL_HANDLE) 
        {
            vkDestroyImage(*device, image, nullptr);
            image = VK_NULL_HANDLE;
        }

        if (image_sampler != VK_NULL_HANDLE) 
        {
            vkDestroySampler(*device, image_sampler, nullptr);
            image_sampler = VK_NULL_HANDLE;
        }

        if (image_memory != VK_NULL_HANDLE) 
        {
            vkFreeMemory(*device, image_memory, nullptr);
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
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device->physical_device, &properties);

        VkSamplerCreateInfo sampler_info { };
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = mag_filter;
        sampler_info.minFilter  = min_filter;
        sampler_info.mipmapMode = mipmap_mode;
        sampler_info.addressModeU = address_mode_U;
        sampler_info.addressModeV = address_mode_V;
        sampler_info.addressModeW = address_mode_W;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        sampler_info.anisotropyEnable = VK_FALSE;

        sampler_info.minLod = 0.0f;
        sampler_info.maxLod =  static_cast<float>(mip_levels);
        sampler_info.mipLodBias = 0.0f;

        vk_check(vkCreateSampler(*device, &sampler_info, nullptr, &image_sampler));

        if (descriptor_info.sampler) 
        {
            vkDestroySampler(*device, descriptor_info.sampler, nullptr);
        }
        descriptor_info.sampler = image_sampler;
    }

    std::shared_ptr<vk_texture> vk_texture::create_2d(vk_device* in_device,VkCommandPool in_pool,const std::string& path)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) 
        {
            LOG_IO_FATAL("加载图片{0}失败！",path);
        }

        auto stageBuffer = vk_buffer::create(
            *in_device,
            in_pool,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            imageSize,
            pixels
        );

        stbi_image_free(pixels);

        auto ret = std::make_shared<vk_texture>(in_device);
        ret->width = texWidth;
        ret->height = texHeight;
        ret->channels = texChannels;

        // + 1 至少有一个mip
        ret->mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        create_image(
            texWidth, 
            texHeight, 
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            ret->image, 
            ret->image_memory,
            *in_device,
            ret->mip_levels
        );

        transition_image_layout(
            ret->image, 
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            in_pool,
            *in_device,
			in_device->graphics_queue,
			ret->mip_levels
        );

        copy_buffer_to_image(
            stageBuffer->buffer, 
            ret->image, 
            static_cast<uint32_t>(texWidth), 
            static_cast<uint32_t>(texHeight),
            in_pool,
            *in_device,
            in_device->graphics_queue
        );

        generate_mipmaps(ret->image,VK_FORMAT_R8G8B8A8_SRGB,texWidth,texHeight,ret->mip_levels,in_pool,*in_device,in_device->graphics_queue,in_device->physical_device);

        ret->image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ret->image_view = create_imageView(&ret->image,VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT,*in_device,ret->mip_levels);
        ret->format = VK_FORMAT_R8G8B8A8_SRGB;
        ret->descriptor_info.sampler = ret->image_sampler;
        ret->descriptor_info.imageLayout = ret->image_layout;
        ret->descriptor_info.imageView = ret->image_view;

        return ret;
    }
}}