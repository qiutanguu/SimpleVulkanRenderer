#pragma once
#include "vk_common.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "../scene_textures.h"

namespace flower { namespace graphics{

	struct vk_renderpass_mix_data
	{
		vk_renderpass_mix_data(
			vk_device* in_device,
			vk_swapchain* in_swapchain
		):
			device(in_device),
			swapchain(in_swapchain)
		{

		}

		~vk_renderpass_mix_data()
		{
	
		}

		vk_device* device;
		vk_swapchain* swapchain;
	};

	class vk_renderpass
	{
	public:
		vk_renderpass(){ }
		virtual ~vk_renderpass(){ }
		virtual void swapchain_change(vk_renderpass_mix_data in_mixdata) = 0;

		VkRenderPass render_pass;
		int32_t type;
	};

	namespace renderpass_type
	{
		constexpr auto texture_pass = 0;
		constexpr auto gbuffer_pass = 1;

		constexpr auto max_index = 2;
	}

} }