#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"
#include "vk/vk_texture.h"
#include "vk/vk_swapchain.h"

namespace flower{ namespace graphics{

	struct global_matrix_vp
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

}}