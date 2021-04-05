#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vk/vk_buffer.h"

namespace flower{ namespace graphics{

	// »ù´¡mvp¾ØÕó
	struct uniform_buffer_mvp
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
}}