#pragma once
#include "vk_common.h"
#include "vk_device.h"
#include "vk_descriptor_set.h"

namespace spirv_cross
{
    class Compiler;
    class ShaderResources;
}

namespace flower { namespace graphics{
	
	struct vk_shader_module
	{
        vk_shader_module(vk_device* in_device) : device(in_device) {  }

        ~vk_shader_module()
        {
            if (handle != VK_NULL_HANDLE) 
            {
                vkDestroyShaderModule(*device, handle, nullptr);
                handle = VK_NULL_HANDLE;
            }
        }

        static std::shared_ptr<vk_shader_module> create(vk_device* in_device, const char* filename, VkShaderStageFlagBits stage);

        vk_device* device;
        VkShaderStageFlagBits stage;
        VkShaderModule handle;
        std::vector<char> shader_code;
	};

	class vk_shader_mix
	{
    public:
        vk_shader_mix(vk_device* in_device, bool dynamic_uniform_buffer) : device(in_device),dynamic_uniform_buffer(dynamic_uniform_buffer) {  }
        ~vk_shader_mix(){ }

        static std::shared_ptr<vk_shader_mix> create(
            vk_device* in_device, 
            bool dynamic_uniform_buffer, 
            const char* vert_path, 
            const char* frag_path, 
            const char* geom_path = nullptr, 
            const char* comp_path = nullptr, 
            const char* tesc_path = nullptr, 
            const char* tese_path = nullptr
        );
    private:
        void parser();
        void parser_shader_module(std::shared_ptr<vk_shader_module> shader);

        void parser_uniform_buffer(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);

    private:
        vk_device* device;
        bool dynamic_uniform_buffer = false;

    public:
        std::shared_ptr<vk_shader_module> vert_shader_module;
        std::shared_ptr<vk_shader_module> frag_shader_module;
        std::shared_ptr<vk_shader_module> geom_shader_module;
        std::shared_ptr<vk_shader_module> comp_shader_module;
        std::shared_ptr<vk_shader_module> tesc_shader_module;
        std::shared_ptr<vk_shader_module> tese_shader_module;

        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
        vk_descriptor_set_layouts_info set_layouts_info;
	};

}}