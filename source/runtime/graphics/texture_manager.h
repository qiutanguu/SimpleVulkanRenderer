#pragma once
#include "vk/vk_common.h"
#include "vk/vk_texture.h"
#include "vk/vk_device.h"
#include <unordered_map>

namespace flower { namespace graphics{

	class sampler_manager
	{
	
	public:
		sampler_manager(){ }
		~sampler_manager(){ }

		void release()
		{
			vkDestroySampler(*device, linear_repeat_no_mip, nullptr);
			vkDestroySampler(*device, nearest_clamp_no_mip, nullptr);
			vkDestroySampler(*device, shadow_depth_pcf_no_mip, nullptr);
			vkDestroySampler(*device,linear_clamp_no_mip,nullptr);
		}

		void initialize(vk_device* in_device)
		{
			device = in_device;
			inner_init();
		}

		VkSampler linear_repeat_no_mip;
		VkSampler linear_clamp_no_mip;
		VkSampler nearest_clamp_no_mip;
		VkSampler shadow_depth_pcf_no_mip;
		

	private:
		void inner_init();
		vk_device* device;
	};

	extern sampler_manager g_sampler_manager;

	// 全局的纹理资源管理器
	class texture_manager
	{
	public:
		texture_manager(){ }
		~texture_manager() {  }

		void release()
		{
			texture_map.clear();
			textures.clear();

			texture_map ={};
			textures = {};
		}

		void initialize(vk_device* in_device,VkCommandPool in_pool);

		uint32_t load_texture_mipmap(VkFormat texture_format,const sampler_layout& in,std::string texture_path);

		uint32_t get_texture_id(std::string texture_path)
		{ 
			check_init();
			return texture_map[texture_path]; 
		}

		std::shared_ptr<vk_texture> get_texture_vk(std::string texture_path)
		{
			check_init();
			return textures[get_texture_id(texture_path)];
		}

		std::shared_ptr<vk_texture> get_texture_vk(uint32_t id)
		{
			check_init();
			return textures[id];
		}

	public:
		std::unordered_map<std::string,uint32_t> texture_map = {};
		std::vector<std::shared_ptr<vk_texture>> textures = {};

		uint32_t white_16x16;
		uint32_t black_16x16;
		uint32_t checkboard;
		uint32_t default_normal;
	private:
		bool has_init = false;
		uint32_t current_id = 0;

		void check_init();


		vk_device* device;
		VkCommandPool pool;
	};

	extern texture_manager g_texture_manager;

} }