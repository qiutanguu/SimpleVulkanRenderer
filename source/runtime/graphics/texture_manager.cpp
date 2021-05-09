#include "texture_manager.h"

namespace flower { namespace graphics{

	texture_manager g_texture_manager = {};
	sampler_manager g_sampler_manager = {};

	uint32_t texture_manager::load_texture_mipmap(
		VkFormat texture_format,
		const sampler_layout& layout,
		std::string texture_path)
	{
		check_init();

		if(texture_map.count(texture_path) > 0)
			return texture_map[texture_path];

		auto res = vk_texture::create_2d_mipmap(device,pool,texture_format,texture_path);

		res->update_sampler(
			layout
		);

		texture_map.insert(std::make_pair(texture_path,current_id));
		textures.push_back(res);
		current_id ++;

		return current_id-1;
	}

	void texture_manager::check_init()
	{
		ASSERT(has_init,"texture manager ÉÐÎ´³õÊ¼»¯£¡");
	}

	void texture_manager::initialize(vk_device* in_device,VkCommandPool in_pool)
	{
		device = in_device;
		pool = in_pool;
		has_init = true;

		white_16x16 = load_texture_mipmap(
			VK_FORMAT_R8G8B8A8_UNORM,
			sampler_layout::linear_repeat(),
			"data/image/white.png"
		);

		black_16x16 = load_texture_mipmap(
			VK_FORMAT_R8G8B8A8_UNORM,
			sampler_layout::linear_repeat(),
			"data/image/black.png"
		);

		checkboard = load_texture_mipmap(
			VK_FORMAT_R8G8B8A8_UNORM,
			sampler_layout::linear_repeat(),
			"data/image/checkerboard.png"
		);

		default_normal = load_texture_mipmap(
			VK_FORMAT_R8G8B8A8_UNORM,
			sampler_layout::linear_repeat(),
			"data/image/default_normal.TGA"
		);
	}

	void fill_sampler_create_info(VkSamplerCreateInfo& sampler_info,const sampler_layout& in,vk_device* device)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device->physical_device,&properties);

		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = in.mag_filter;
		sampler_info.minFilter = in.min_filter;
		sampler_info.mipmapMode = in.mipmap_mode;
		sampler_info.addressModeU = in.address_mode_U;
		sampler_info.addressModeV = in.address_mode_V;
		sampler_info.addressModeW = in.address_mode_W;
		sampler_info.compareOp = in.compare_op;
		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareEnable = in.compareEnable;
		sampler_info.borderColor = in.bordercolor;
		sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		sampler_info.anisotropyEnable = VK_FALSE;

		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;
		sampler_info.mipLodBias = 0.0f;
	}

	void sampler_manager::inner_init()
	{
		VkSamplerCreateInfo sampler_info{ };

		fill_sampler_create_info(sampler_info,sampler_layout::nearset_clamp(),device);
		vk_check(vkCreateSampler(*device,&sampler_info,nullptr,&nearest_clamp_no_mip));

		fill_sampler_create_info(sampler_info,sampler_layout::linear_repeat(),device);
		vk_check(vkCreateSampler(*device,&sampler_info,nullptr,&linear_repeat_no_mip));

		fill_sampler_create_info(sampler_info,sampler_layout::shadow_depth_pcf(),device);
		vk_check(vkCreateSampler(*device,&sampler_info,nullptr,&shadow_depth_pcf_no_mip));

		fill_sampler_create_info(sampler_info,sampler_layout::linear_clamp(),device);
		vk_check(vkCreateSampler(*device,&sampler_info,nullptr,&linear_clamp_no_mip));

	}
}}