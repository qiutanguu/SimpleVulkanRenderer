#include "base_pass.h"

namespace flower{ namespace graphics{

	std::shared_ptr<base_pass> base_pass::create(vk_device* device,vk_depth_resource& depth_resource,vk_swapchain& swapchain)
	{
		auto ret = std::make_shared<base_pass>(device,depth_resource,swapchain);



		return ret;
	}

}}
