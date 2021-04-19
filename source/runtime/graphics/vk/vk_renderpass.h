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
			vk_swapchain& in_swapchain
		):
			device(in_device),
			swapchain(in_swapchain)
		{

		}

		~vk_renderpass_mix_data()
		{
	
		}

		vk_device* device;
		vk_swapchain& swapchain;
	};

} }