#include "vk_shader.h"
#include "spirv_cross/spirv_cross.hpp"

namespace flower { namespace graphics{

	std::shared_ptr<vk_shader_module> vk_shader_module::create(
		vk_device* in_device,
		const char* filename,
		VkShaderStageFlagBits stage)
	{
        auto ret = std::make_shared<vk_shader_module>(in_device);
        ret->shader_code = read_file_binary(filename);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = ret->shader_code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(ret->shader_code.data());

		if (vkCreateShaderModule(*in_device, &createInfo, nullptr, &ret->handle) != VK_SUCCESS) 
		{
			LOG_VULKAN_ERROR("´´½¨shaderÄ£¿é{0}Ê§°Ü£¡",filename);
		}

		ret->stage = stage;
        return ret;
	}

	std::shared_ptr<vk_shader_mix> vk_shader_mix::create(
		vk_device* in_device,
		bool dynamic_uniform_buffer,
		const char* vert,
		const char* frag,
		const char* geom,
		const char* comp,
		const char* tesc,
		const char* tese)
	{
		auto ret =  std::make_shared<vk_shader_mix>(in_device,dynamic_uniform_buffer);

		ret->vert_shader_module = vert ? vk_shader_module::create(in_device, vert, VK_SHADER_STAGE_VERTEX_BIT)   : nullptr;
		ret->frag_shader_module = frag ? vk_shader_module::create(in_device, frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
		ret->geom_shader_module = geom ? vk_shader_module::create(in_device, geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
		ret->comp_shader_module = comp ? vk_shader_module::create(in_device, comp, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;
		ret->tesc_shader_module = tesc ? vk_shader_module::create(in_device, tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)    : nullptr;
		ret->tese_shader_module = tese ? vk_shader_module::create(in_device, tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;

		// ret->parser();

		return ret;
	}
}}