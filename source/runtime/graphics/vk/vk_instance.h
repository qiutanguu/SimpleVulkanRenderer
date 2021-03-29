#pragma once
#include <string>
#include <unordered_map>
#include "core/core.h"
#include <vulkan/vulkan.h>

namespace flower { namespace graphics{

	enum class vk_version
	{
		vk_1_0,
		vk_1_1,
		vk_1_2,
	};

	class vk_instance
	{
	public:
		vk_instance(){ }
		~vk_instance(){ }

		operator VkInstance()
		{
			return instance;
		}

		void initialize(
			const std::vector<const char *> &required_extensions,
			const std::vector<const char *>& required_validation_layers,
			vk_version version,
			const std::string& application_name = "flower");

		void destroy();



		// 插件是否开启
		inline bool extension_is_enabled(const char *extension) const 
		{ 
			return std::find_if(enable_exts.begin(),enable_exts.end(),
				[extension](const char *enabled_extension)
			{
				return strcmp(extension,enabled_extension)==0;
			})!=enable_exts.end(); 
		}

		// 获取启用的插件
		const std::vector<const char*>& GetExtensions(){ return enable_exts; }

		VkInstance instance;
	private:
		std::vector<const char *> enable_exts;

#if defined(FLOWER_DEBUG) 
		VkDebugUtilsMessengerEXT debugUtilsMessenger;
		VkDebugReportCallbackEXT debugReportCallback;
#endif
	};

}}