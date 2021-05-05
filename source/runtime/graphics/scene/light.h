#pragma once
#include "../vk/vk_common.h"

namespace flower{ namespace graphics{

	struct directional_light
	{
		glm::vec4 direction;
		glm::vec4 color;
		glm::vec4 shadow_mix;
	};

} }