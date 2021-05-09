#include "vk_shader.h"

#pragma warning(push)
#pragma warning(disable:4099)
#include "spirv_cross/spirv_cross.hpp"
#pragma warning(pop)

namespace flower { namespace graphics{

	vk_shader_mix::~vk_shader_mix()
	{
		vert_shader_module.reset();
		frag_shader_module.reset();
		geom_shader_module.reset();
		comp_shader_module.reset();
		tesc_shader_module.reset();
		tese_shader_module.reset();

		for(int32_t i = 0; i < shader_descriptor_set_layouts.size(); ++i)
		{
			vkDestroyDescriptorSetLayout(*device,shader_descriptor_set_layouts[i],nullptr);
		}
		shader_descriptor_set_layouts.clear();

		if( pipeline_layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(*device,pipeline_layout,nullptr);
			pipeline_layout = VK_NULL_HANDLE;
		}

		descriptor_set_pools.resize(0);
	}

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
			LOG_VULKAN_ERROR("创建shader模块{0}失败！",filename);
		}

		ret->stage = stage;

		ret->stage_create_info = {};
		ret->stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ret->stage_create_info.stage = stage;
		ret->stage_create_info.module = ret->handle;
		ret->stage_create_info.pName = "main";

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

		ret->parser();

		return ret;
	}

	std::shared_ptr<vk_descriptor_set> vk_shader_mix::allocate_descriptor_set()
	{
		if(set_layouts_info.set_layouts.size() == 0 )
		{
			return nullptr;
		}

		auto dvkSet = std::make_shared<vk_descriptor_set>(device);

		dvkSet->set_layouts_info = set_layouts_info;
		dvkSet->descriptor_sets.resize(set_layouts_info.set_layouts.size());

		for(int32_t i = (int32_t)descriptor_set_pools.size() - 1; i>=0; --i)
		{
			if(descriptor_set_pools[i]->allocate_descriptor_set(dvkSet->descriptor_sets.data()))
			{
				return dvkSet;
			}
		}

		auto set_pool = std::make_shared<vk_descriptor_set_pool>(device,64,set_layouts_info,shader_descriptor_set_layouts);

		descriptor_set_pools.push_back(set_pool);
		set_pool->allocate_descriptor_set(dvkSet->descriptor_sets.data());

		return dvkSet;
	}

	void vk_shader_mix::parser()
	{
		parser_shader_module(vert_shader_module);
		parser_shader_module(frag_shader_module);
		parser_shader_module(geom_shader_module);
		parser_shader_module(comp_shader_module);
		parser_shader_module(tesc_shader_module);
		parser_shader_module(tese_shader_module);

		generate_input_info();
		generate_layout();
	}

	void vk_shader_mix::parser_shader_module(std::shared_ptr<vk_shader_module> shader_module)
	{
		if(!shader_module)
		{
			return;
		}

		// 1. 存储shader模块信息。
		VkPipelineShaderStageCreateInfo shaderCreateInfo { };
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderCreateInfo.stage = shader_module->stage;
		shaderCreateInfo.module = shader_module->handle;
		shaderCreateInfo.pName = "main";
		shader_stage_create_infos.push_back(shaderCreateInfo);

		// 2. 获取spv反射信息。
		spirv_cross::Compiler compiler((uint32_t*)shader_module->shader_code.data(),shader_module->shader_code.size()/sizeof(uint32_t));
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		process_attachments(compiler,resources,shader_module->stage);
		process_uiform_buffers(compiler,resources,shader_module->stage);
		process_textures(compiler,resources,shader_module->stage);
		process_storage_images(compiler,resources,shader_module->stage);
		process_input(compiler,resources,shader_module->stage);
		process_storage_buffers(compiler,resources,shader_module->stage);
	}

	void vk_shader_mix::process_attachments(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkPipelineStageFlags stageFlags)
	{
		for(int32_t i = 0; i < resources.subpass_inputs.size(); ++i)
		{
			spirv_cross::Resource& res = resources.subpass_inputs[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string& varName = compiler.get_name(res.id);

			int32_t set = compiler.get_decoration(res.id,spv::DecorationDescriptorSet);
			int32_t binding = compiler.get_decoration(res.id,spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			setLayoutBinding.descriptorCount = 1;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			set_layouts_info.add_descriptor_set_layout_binding(varName,set,setLayoutBinding);

			auto it = image_params.find(varName);
			if( it == image_params.end())
			{
				image_info imageInfo = {};
				imageInfo.set = set;
				imageInfo.binding = binding;
				imageInfo.stage_flags = stageFlags;
				imageInfo.descriptor_type = setLayoutBinding.descriptorType;
				image_params.insert(std::make_pair(varName,imageInfo));
			}
			else
			{
				it->second.stage_flags |= stageFlags;
			}
		}
	}

	void vk_shader_mix::process_uiform_buffers(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
	{
		for(int32_t i = 0; i<resources.uniform_buffers.size(); ++i)
		{
			spirv_cross::Resource& res = resources.uniform_buffers[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string& varName = compiler.get_name(res.id);
			const std::string& typeName = compiler.get_name(res.base_type_id);
			uint32_t uniformBufferStructSize = (uint32_t)compiler.get_declared_struct_size(type);

			int32_t set = compiler.get_decoration(res.id,spv::DecorationDescriptorSet);
			int32_t binding = compiler.get_decoration(res.id,spv::DecorationBinding);

			// _dynamic 标记的 uniform buffer 为 dynamic uniform buffer
			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorType = (typeName.find("_dynamic") != std::string::npos || dynamic_uniform_buffer) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setLayoutBinding.descriptorCount = 1;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			set_layouts_info.add_descriptor_set_layout_binding(varName,set,setLayoutBinding);

			// 保存 uniform buffer 变量信息
			auto it = buffer_params.find(varName);
			if(it==buffer_params.end())
			{
				buffer_info bufferInfo = {};
				bufferInfo.set = set;
				bufferInfo.binding = binding;
				bufferInfo.buffer_size = uniformBufferStructSize;
				bufferInfo.stage_flags = stageFlags;
				bufferInfo.descriptor_type = setLayoutBinding.descriptorType;
				buffer_params.insert(std::make_pair(varName,bufferInfo));
			}
			else
			{
				it->second.stage_flags |= setLayoutBinding.stageFlags;
			}
		}
	}

	void vk_shader_mix::process_textures(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
	{
		for(int32_t i = 0; i<resources.sampled_images.size(); ++i)
		{
			spirv_cross::Resource& res = resources.sampled_images[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string& varName = compiler.get_name(res.id);

			int32_t set = compiler.get_decoration(res.id,spv::DecorationDescriptorSet);
			int32_t binding = compiler.get_decoration(res.id,spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			setLayoutBinding.descriptorCount = 1;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			set_layouts_info.add_descriptor_set_layout_binding(varName,set,setLayoutBinding);

			auto it = image_params.find(varName);
			if(it==image_params.end())
			{
				image_info imageInfo = {};
				imageInfo.set = set;
				imageInfo.binding = binding;
				imageInfo.stage_flags = stageFlags;
				imageInfo.descriptor_type = setLayoutBinding.descriptorType;
				image_params.insert(std::make_pair(varName,imageInfo));
			}
			else
			{
				it->second.stage_flags |= stageFlags;
			}
		}
	}

	vertex_attribute string_to_vertex_attribute(const char* name)
	{
		if(strcmp(name,"in_pos")==0)
		{
			return vertex_attribute::pos;
		}
		else if(strcmp(name,"in_uv0")==0)
		{
			return vertex_attribute::uv0;
		}
		else if(strcmp(name,"in_uv1")==0)
		{
			return vertex_attribute::uv1;
		}
		else if(strcmp(name,"in_normal")==0)
		{
			return vertex_attribute::normal;
		}
		else if(strcmp(name,"in_color")==0)
		{
			return vertex_attribute::color;
		}
		else if(strcmp(name,"in_alpha")==0)
		{
			return vertex_attribute::alpha;
		}
		else if(strcmp(name,"in_tangent")==0)
		{
			return vertex_attribute::tangent;
		}
		else if(strcmp(name,"in_skin_index")==0)
		{
			return vertex_attribute::skin_index;
		}
		else if(strcmp(name,"in_skin_pack")==0)
		{
			return vertex_attribute::skin_pack;
		}
		else if(strcmp(name,"in_skin_weight")==0)
		{
			return vertex_attribute::skin_weight;
		}
		else if(strcmp(name,"in_uv2")==0)
		{
			return vertex_attribute::uv2;
		}
		else if(strcmp(name,"in_uv3")==0)
		{
			return vertex_attribute::uv3;
		}
		else if(strcmp(name,"in_uv4")==0)
		{
			return vertex_attribute::uv4;
		}
		else if(strcmp(name,"in_uv5")==0)
		{
			return vertex_attribute::uv5;
		}
		else if(strcmp(name,"in_uv6")==0)
		{
			return vertex_attribute::uv6;
		}

		return vertex_attribute::none;
	}

	void vk_shader_mix::process_input(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
	{
		if(stageFlags!=VK_SHADER_STAGE_VERTEX_BIT)
		{
			return;
		}

		// 获取input信息
		for(int32_t i = 0; i<resources.stage_inputs.size(); ++i)
		{
			spirv_cross::Resource& res = resources.stage_inputs[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			const std::string& varName = compiler.get_name(res.id);
			int32_t inputAttributeSize = type.vecsize;

			vertex_attribute attribute = string_to_vertex_attribute(varName.c_str());
			if(attribute == vertex_attribute::none)
			{
				if(inputAttributeSize==1)
				{
					attribute = vertex_attribute::instance_float;
				}
				else if(inputAttributeSize==2)
				{
					attribute = vertex_attribute::instance_vec2;
				}
				else if(inputAttributeSize==3)
				{
					attribute = vertex_attribute::instance_vec3;
				}
				else if(inputAttributeSize==4)
				{
					attribute = vertex_attribute::instance_vec4;
				}

				LOG_VULKAN_ERROR("找不到属性：{0}, 将其视为实例属性 : {1}.",varName.c_str(),int32_t(attribute));
			}

			int32_t location = compiler.get_decoration(res.id,spv::DecorationLocation);

			input_attribute attri = {};
			attri.location = location;
			attri.attribute = attribute;
			inner_attributes.push_back(attri);
		}
	}

	void vk_shader_mix::process_storage_buffers(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
	{
		for(int32_t i = 0; i<resources.storage_buffers.size(); ++i)
		{
			spirv_cross::Resource& res = resources.storage_buffers[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string& varName = compiler.get_name(res.id);

			int32_t set = compiler.get_decoration(res.id,spv::DecorationDescriptorSet);
			int32_t binding = compiler.get_decoration(res.id,spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setLayoutBinding.descriptorCount = 1;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			set_layouts_info.add_descriptor_set_layout_binding(varName,set,setLayoutBinding);

			auto it = buffer_params.find(varName);
			if(it==buffer_params.end())
			{
				buffer_info bufferInfo = {};
				bufferInfo.set = set;
				bufferInfo.binding = binding;
				bufferInfo.buffer_size = 0;
				bufferInfo.stage_flags = stageFlags;
				bufferInfo.descriptor_type = setLayoutBinding.descriptorType;
				buffer_params.insert(std::make_pair(varName,bufferInfo));
			}
			else
			{
				it->second.stage_flags = it->second.stage_flags | setLayoutBinding.stageFlags;
			}
		}
	}

	void vk_shader_mix::process_storage_images(spirv_cross::Compiler& compiler,spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
	{
		for(int32_t i = 0; i<resources.storage_images.size(); ++i)
		{
			spirv_cross::Resource& res = resources.storage_images[i];
			spirv_cross::SPIRType type = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string& varName = compiler.get_name(res.id);

			int32_t set = compiler.get_decoration(res.id,spv::DecorationDescriptorSet);
			int32_t binding = compiler.get_decoration(res.id,spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			setLayoutBinding.descriptorCount = 1;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			set_layouts_info.add_descriptor_set_layout_binding(varName,set,setLayoutBinding);

			auto it = image_params.find(varName);
			if(it==image_params.end())
			{
				image_info imageInfo = {};
				imageInfo.set = set;
				imageInfo.binding = binding;
				imageInfo.stage_flags = stageFlags;
				imageInfo.descriptor_type = setLayoutBinding.descriptorType;
				image_params.insert(std::make_pair(varName,imageInfo));
			}
			else
			{
				it->second.stage_flags |= stageFlags;
			}
		}
	}

	void vk_shader_mix::generate_input_info()
	{
		// 1. 对inputAttributes进行排序，获取Attributes列表
		std::sort(inner_attributes.begin(),inner_attributes.end(),[](const input_attribute& a,const input_attribute& b) -> bool
		{
			return a.location < b.location;
		});

		for(int32_t i = 0; i < inner_attributes.size(); ++i)
		{
			vertex_attribute attribute = inner_attributes[i].attribute;

			if( attribute == vertex_attribute::instance_float ||
				attribute == vertex_attribute::instance_vec2  ||
				attribute == vertex_attribute::instance_vec3  ||
				attribute == vertex_attribute::instance_vec4)
			{
				instances_attributes.push_back(attribute);
			}
			else
			{
				per_vertex_attributes.push_back(attribute);
			}
		}

		// 2. 生成 binding info
		input_bindings.resize(0);
		if(per_vertex_attributes.size()>0)
		{
			int32_t stride = 0;
			for(int32_t i = 0; i<per_vertex_attributes.size(); ++i)
			{
				stride += vertex_attribute_size(per_vertex_attributes[i]);
			}
			VkVertexInputBindingDescription perVertexInputBinding = {};
			perVertexInputBinding.binding = 0;
			perVertexInputBinding.stride = stride;
			perVertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			input_bindings.push_back(perVertexInputBinding);
		}

		if(instances_attributes.size()>0)
		{
			int32_t stride = 0;
			for(int32_t i = 0; i<instances_attributes.size(); ++i)
			{
				stride += vertex_attribute_size(instances_attributes[i]);
			}
			VkVertexInputBindingDescription instanceInputBinding = {};
			instanceInputBinding.binding = 1;
			instanceInputBinding.stride = stride;
			instanceInputBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			input_bindings.push_back(instanceInputBinding);
		}

		// 3. 生成 attributes info
		int location = 0;
		if(per_vertex_attributes.size()>0)
		{
			int32_t offset = 0;
			for(int32_t i = 0; i<per_vertex_attributes.size(); ++i)
			{
				VkVertexInputAttributeDescription inputAttribute = {};
				inputAttribute.binding = 0;
				inputAttribute.location = location;
				inputAttribute.format = VertexAttributeToVkFormat(per_vertex_attributes[i]);
				inputAttribute.offset = offset;
				offset += vertex_attribute_size(per_vertex_attributes[i]);
				input_attributes.push_back(inputAttribute);

				location += 1;
			}
		}

		if(instances_attributes.size()>0)
		{
			int32_t offset = 0;
			for(int32_t i = 0; i<instances_attributes.size(); ++i)
			{
				VkVertexInputAttributeDescription inputAttribute = {};
				inputAttribute.binding = 1;
				inputAttribute.location = location;
				inputAttribute.format = VertexAttributeToVkFormat(instances_attributes[i]);
				inputAttribute.offset = offset;
				offset += vertex_attribute_size(instances_attributes[i]);
				input_attributes.push_back(inputAttribute);

				location += 1;
			}
		}
	}

	void vk_shader_mix::generate_layout()
	{
		std::vector<vk_descriptor_set_layout_info>& setLayouts = set_layouts_info.set_layouts;

		// 1. 先按照set进行排序
		std::sort(setLayouts.begin(),setLayouts.end(),[](const vk_descriptor_set_layout_info& a,const vk_descriptor_set_layout_info& b) -> bool
		{
			return a.set < b.set;
		});

		// 2. 再按照binding进行排序
		for(int32_t i = 0; i < setLayouts.size(); ++i)
		{
			std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
			std::sort(bindings.begin(),bindings.end(),[](const VkDescriptorSetLayoutBinding& a,const VkDescriptorSetLayoutBinding& b) -> bool
			{
				return a.binding<b.binding;
			});
		}

		for(int32_t i = 0; i < set_layouts_info.set_layouts.size(); ++i)
		{
			VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
			vk_descriptor_set_layout_info& setLayoutInfo = set_layouts_info.set_layouts[i];

			VkDescriptorSetLayoutCreateInfo descSetLayoutInfo{ };
			descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetLayoutInfo.bindingCount = (uint32_t)setLayoutInfo.bindings.size();
			descSetLayoutInfo.pBindings = setLayoutInfo.bindings.data();
			vk_check(vkCreateDescriptorSetLayout(*device,&descSetLayoutInfo,nullptr,&descriptorSetLayout));

			shader_descriptor_set_layouts.push_back(descriptorSetLayout);
		}

		VkPipelineLayoutCreateInfo pipeLayoutInfo { };
		pipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeLayoutInfo.setLayoutCount = (uint32_t)shader_descriptor_set_layouts.size();
		pipeLayoutInfo.pSetLayouts = shader_descriptor_set_layouts.data();
		vk_check(vkCreatePipelineLayout(*device,&pipeLayoutInfo,nullptr,&pipeline_layout));
	}
}}