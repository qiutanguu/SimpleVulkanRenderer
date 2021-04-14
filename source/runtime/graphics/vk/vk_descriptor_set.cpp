#include "vk_descriptor_set.h"

namespace flower{ namespace graphics{

	VkDescriptorType vk_descriptor_set_layouts_info::get_descriptor_type(int32_t set,int32_t binding)
	{
		for(int32_t i = 0;  i < set_layouts.size(); i++)
		{
			if(set_layouts[i].set == set)
			{
				for(int32_t j = 0; j < set_layouts[i].bindings.size(); ++j)
				{
					if(set_layouts[i].bindings[j].binding == binding)
					{
						return set_layouts[i].bindings[j].descriptorType;
					}
				}
			}
		}

		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	}

	void vk_descriptor_set_layouts_info::add_descriptor_set_layout_binding(const std::string& name,int32_t set,VkDescriptorSetLayoutBinding binding)
	{
		vk_descriptor_set_layout_info* set_layout = nullptr;

		for(int32_t i = 0; i < set_layouts.size(); ++i)
		{
			if(set_layouts[i].set==set)
			{
				set_layout = &(set_layouts[i]);
				break;
			}
		}

		if(set_layout == nullptr)
		{
			set_layouts.push_back({ });
			set_layout = &(set_layouts[set_layouts.size() - 1]);
		}

		for(int32_t i = 0; i < set_layout->bindings.size(); i++)
		{
			VkDescriptorSetLayoutBinding& set_binding = set_layout->bindings[i];
			if(set_binding.binding==binding.binding && set_binding.descriptorType == binding.descriptorType)
			{
				set_binding.stageFlags = set_binding.stageFlags | binding.stageFlags;
				return;
			}
		}

		set_layout->set = set;
		set_layout->bindings.push_back(binding);

		bind_info param_info = {};
		param_info.set = set;
		param_info.binding = binding.binding;
		params_map.insert(std::make_pair(name,param_info));
	}

	void vk_descriptor_set::set_image(const std::string& name,std::shared_ptr<vk_texture> texture)
	{
		auto it = set_layouts_info.params_map.find(name);
		if(it == set_layouts_info.params_map.end())
		{
			LOG_VULKAN_ERROR("图片Buffer写入失败！{0}没有找到",name.c_str());
			return;
		}

		auto bind_info = it->second;

		VkWriteDescriptorSet write_descriptor_set { };
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		write_descriptor_set.dstSet = descriptor_sets[bind_info.set];
		write_descriptor_set.descriptorCount = 1;
		write_descriptor_set.descriptorType = set_layouts_info.get_descriptor_type(bind_info.set,bind_info.binding);
		write_descriptor_set.pBufferInfo = nullptr;
		write_descriptor_set.pImageInfo = &(texture->descriptor_info);
		write_descriptor_set.dstBinding = bind_info.binding;
		vkUpdateDescriptorSets(*device,1,&write_descriptor_set,0,nullptr);
	}

	void vk_descriptor_set::set_buffer(const std::string& name,std::shared_ptr<vk_buffer> buffer)
	{
		auto it = set_layouts_info.params_map.find(name);
		if(it==set_layouts_info.params_map.end())
		{
			LOG_VULKAN_ERROR("Buffer写入失败！{0}没有找到",name.c_str());
			return;
		}

		auto bind_info = it->second;

		VkWriteDescriptorSet write_descriptor_set{ };
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.dstSet = descriptor_sets[bind_info.set];
		write_descriptor_set.descriptorCount = 1;
		write_descriptor_set.descriptorType = set_layouts_info.get_descriptor_type(bind_info.set,bind_info.binding);
		write_descriptor_set.pBufferInfo = &(buffer->descriptor);
		write_descriptor_set.dstBinding = bind_info.binding;
		vkUpdateDescriptorSets(*device,1,&write_descriptor_set,0,nullptr);
	}

	vk_descriptor_set_pool::vk_descriptor_set_pool(
		vk_device* in_device,
		int32_t in_max_set,
		const vk_descriptor_set_layouts_info& set_layouts_info,
		const std::vector<VkDescriptorSetLayout>& in_descriptor_set_layouts)
		: device(in_device), max_set(in_max_set), used_set(0),descriptor_set_layouts(in_descriptor_set_layouts)
	{

		std::vector<VkDescriptorPoolSize> pool_sizes;
		for(int32_t i = 0; i < set_layouts_info.set_layouts.size(); ++i)
		{
			const vk_descriptor_set_layout_info& set_layout_info = set_layouts_info.set_layouts[i];
			for(int32_t j = 0; j < set_layout_info.bindings.size(); ++j)
			{
				VkDescriptorPoolSize pool_size = {};
				pool_size.type = set_layout_info.bindings[j].descriptorType;
				pool_size.descriptorCount = (set_layout_info.bindings[j].descriptorCount) * max_set;
				pool_sizes.push_back(pool_size);
			}
		}

		VkDescriptorPoolCreateInfo descriptor_pool_info {};
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
		descriptor_pool_info.pPoolSizes = pool_sizes.data();
		descriptor_pool_info.maxSets = max_set;
		vk_check(vkCreateDescriptorPool(*device,&descriptor_pool_info,nullptr,&pool));
	}

	vk_descriptor_set_pool::~vk_descriptor_set_pool()
	{
		if(pool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(*device,pool,nullptr);
			pool = VK_NULL_HANDLE;
		}
	}

	bool vk_descriptor_set_pool::allocate_descriptor_set(VkDescriptorSet* descriptorSet)
	{
		if(used_set + descriptor_set_layouts.size() >= max_set)
		{
			return false;
		}

		used_set += (int32_t)descriptor_set_layouts.size();

		VkDescriptorSetAllocateInfo allocInfo { };
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = (uint32_t)descriptor_set_layouts.size();
		allocInfo.pSetLayouts = descriptor_set_layouts.data();
		vk_check(vkAllocateDescriptorSets(*device,&allocInfo,descriptorSet));

		return true;
	}

}}


