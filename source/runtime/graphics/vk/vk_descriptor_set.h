#pragma once
#include "vk_common.h"
#include "vk_vertex_buffer.h"
#include "vk_device.h"
#include "vk_buffer.h"
#include "vk_texture.h"

namespace flower{ namespace graphics{

	struct vk_descriptor_set_layout_info
	{
		int32_t set = -1;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	struct vk_descriptor_set_layouts_info
	{
		struct bind_info
		{
			int32_t set;
			int32_t binding;
		};

		VkDescriptorType get_descriptor_type(int32_t set, int32_t binding);
		void add_descriptor_set_layout_binding(const std::string& varName, int32_t set, VkDescriptorSetLayoutBinding binding);

		std::unordered_map<std::string, bind_info> params_map;
		std::vector<vk_descriptor_set_layout_info> set_layouts;
	};

	struct input_attribute
	{
		vertex_attribute attribute;
		int32_t	location;
	};

	class vk_descriptor_set
	{
	public:
		vk_descriptor_set(vk_device* in_device) : device(in_device) { }
		~vk_descriptor_set(){ }

		void set_image(const std::string& name,std::shared_ptr<vk_texture> texture);
		void set_buffer(const std::string& name, std::shared_ptr<vk_buffer> buffer);

	private:
		vk_device* device;

	public:
		vk_descriptor_set_layouts_info set_layouts_info;
		std::vector<VkDescriptorSet> descriptor_sets;
	};

	class vk_descriptor_set_pool
	{
	public:
		vk_descriptor_set_pool(vk_device* in_device, int32_t in_max_set, const vk_descriptor_set_layouts_info& set_layouts_info, const std::vector<VkDescriptorSetLayout>& in_descriptor_set_layouts);
		
		~vk_descriptor_set_pool();

		bool full() { return used_set >= max_set; }

		bool allocate_descriptor_set(VkDescriptorSet* descriptorSet);
	private:
		vk_device* device;

	public:
		int32_t	max_set;
		int32_t	used_set;
		std::vector<VkDescriptorSetLayout>	descriptor_set_layouts;

		VkDescriptorPool pool;
	};

} }