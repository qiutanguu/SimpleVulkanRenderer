#include "vk_instance.h"
#include "vk_common.h"
#include <glfw/glfw3.h>

namespace flower {namespace graphics{

#ifdef FLOWER_DEBUG

	// 调试打印信息
	VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		void* user_data)
	{
		if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_VULKAN_WARN("{} - {}: {}",callback_data->messageIdNumber,callback_data->pMessageIdName,callback_data->pMessage);
		}
		else if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_VULKAN_ERROR("{} - {}: {}",callback_data->messageIdNumber,callback_data->pMessageIdName,callback_data->pMessage);
		}
		return VK_FALSE;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT /*type*/,
		uint64_t /*object*/,size_t /*location*/,int32_t /*message_code*/,
		const char *layer_prefix,const char *message,void * /*user_data*/)
	{
		if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			LOG_VULKAN_ERROR("{}: {}",layer_prefix,message);
		}
		else if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			LOG_VULKAN_WARN("{}: {}",layer_prefix,message);
		}
		else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			LOG_VULKAN_WARN("{}: {}",layer_prefix,message);
		}
		else
		{
			LOG_VULKAN_INFO("{}: {}",layer_prefix,message);
		}
		return VK_FALSE;
	}

	VkResult create_debug_Utils_messenger_EXT(VkInstance instance,const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
		if(func!=nullptr)
		{
			return func(instance,pCreateInfo,pAllocator,pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void destroy_debug_Utils_messenger_EXT(VkInstance instance,VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
		if(func!=nullptr)
		{
			func(instance,debugMessenger,pAllocator);
		}
	}

	VkResult create_debug_report_callback_EXT(VkInstance instance,const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDebugReportCallbackEXT* pDebugReporter)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugReportCallbackEXT");
		if(func!=nullptr)
		{
			return func(instance,pCreateInfo,pAllocator,pDebugReporter);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void destroy_debug_report_callback_EXT(VkInstance instance,VkDebugReportCallbackEXT DebugReporter,const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugReportCallbackEXT");
		if(func!=nullptr)
		{
			func(instance,DebugReporter,pAllocator);
		}
	}
#endif

	// 检查要求的验证层是否可用
	bool validate_request_layers_available(const std::vector<const char *>& required,const std::vector<VkLayerProperties> &available)
	{
		for(auto layer:required)
		{
			bool found = false;
			for(auto &available_layer:available)
			{
				if(strcmp(available_layer.layerName,layer)==0)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				LOG_VULKAN_ERROR ("未发现验证层：{}",layer);
				return false;
			}
		}
		return true;
	}

	// 选择最佳的验证层
	std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties> &supported_instance_layers)
	{
		std::vector<std::vector<const char *>> validation_layer_priority_list =
		{
			{"VK_LAYER_KHRONOS_validation"},
			{"VK_LAYER_LUNARG_standard_validation"},
			{
				"VK_LAYER_GOOGLE_threading",
				"VK_LAYER_LUNARG_parameter_validation",
				"VK_LAYER_LUNARG_object_tracker",
				"VK_LAYER_LUNARG_core_validation",
				"VK_LAYER_GOOGLE_unique_objects",
			},
			{"VK_LAYER_LUNARG_core_validation"}
		};

		for(auto &validation_layers:validation_layer_priority_list)
		{
			if(validate_request_layers_available(validation_layers,supported_instance_layers))
			{
				return validation_layers;
			}

			LOG_VULKAN_WARN("无法开启验证层！Vulkan将在没有调试讯息情况下启动");
		}

		return {};
	}


	void vk_instance::initialize(
		const std::vector<const char*>& required_extensions,
		const std::vector<const char*>& required_validation_layers,
		vk_version version,
		const std::string & application_name)
	{
		// 1. 查询支持的实例插件
		uint32_t instance_extension_count;
		vk_check(vkEnumerateInstanceExtensionProperties(nullptr,&instance_extension_count,nullptr));
		std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
		vk_check(vkEnumerateInstanceExtensionProperties(nullptr,&instance_extension_count,available_instance_extensions.data()));

#if defined(FLOWER_DEBUG) 
		// 2. Debug插件开启
		bool debug_utils = false;
		for(auto &available_extension:available_instance_extensions)
		{
			if(strcmp(available_extension.extensionName,VK_EXT_DEBUG_UTILS_EXTENSION_NAME)==0)
			{
				debug_utils = true;
				LOG_VULKAN_INFO("插件{}可用！正在开启！",VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				enable_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}
		if(!debug_utils)
		{
			enable_exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
#endif
		
		// 增加表面显示插件
		enable_exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		// 添加glfw插件
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		for(uint32_t i = 0; i<glfwExtensionCount; i++)
		{
			enable_exts.push_back(extensions[i]);
		}

		// 增加其他可用插件
		for(auto &available_extension:available_instance_extensions)
		{
			if(strcmp(available_extension.extensionName,VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)==0)
			{
				LOG_VULKAN_INFO("插件{0}可用！正在开启！",VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
				enable_exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			}
		}

		// 3. 检查要求的插件是否可用
		auto extension_error = false;
		for(auto extension:required_extensions)
		{
			auto extension_name = extension;
			if(std::find_if(available_instance_extensions.begin(),available_instance_extensions.end(),
				[&extension_name](VkExtensionProperties available_extension)
			{
				return strcmp(available_extension.extensionName,extension_name)==0;
			})==available_instance_extensions.end())
			{
				LOG_VULKAN_FATAL("实例插件{0}不可用，无法运行！",extension_name);
				extension_error = true;
			}
			else
			{
				enable_exts.push_back(extension_name);
			}
		}

		if(extension_error)
		{
			LOG_VULKAN_FATAL("丢失请求的实例插件！");
		}

		uint32_t instance_layer_count;
		vk_check(vkEnumerateInstanceLayerProperties(&instance_layer_count,nullptr));

		std::vector<VkLayerProperties> supported_validation_layers(instance_layer_count);
		vk_check(vkEnumerateInstanceLayerProperties(&instance_layer_count,supported_validation_layers.data()));

		std::vector<const char *> requested_validation_layers(required_validation_layers);

#ifdef FLOWER_DEBUG
		std::vector<const char *> optimal_validation_layers = GetOptimalValidationLayers(supported_validation_layers);
		requested_validation_layers.insert(requested_validation_layers.end(),optimal_validation_layers.begin(),optimal_validation_layers.end());
#endif

		if(validate_request_layers_available(requested_validation_layers,supported_validation_layers))
		{
			LOG_VULKAN_INFO("开启的验证层:");
			for(const auto &layer:requested_validation_layers)
			{
				LOG_VULKAN_INFO("	\t{}",layer);
			}
		}
		else
		{
			LOG_VULKAN_FATAL("找不到请求的验证层！");
		}

		VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};

		app_info.pApplicationName = application_name.c_str();
		app_info.applicationVersion = 0;
		app_info.pEngineName = "flower_engine";
		app_info.engineVersion = 0;

		switch(version)
		{
		case flower::graphics::vk_version::vk_1_0:
			app_info.apiVersion = VK_MAKE_VERSION(1,0,0);
			break;
		case flower::graphics::vk_version::vk_1_1:
			app_info.apiVersion = VK_MAKE_VERSION(1,1,0);
			break;
		case flower::graphics::vk_version::vk_1_2:
			app_info.apiVersion = VK_MAKE_VERSION(1,2,0);
			break;
		default:
			app_info.apiVersion = VK_MAKE_VERSION(1,0,0);
			break;
		}
		
		
		VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

		instance_info.pApplicationInfo = &app_info;
		instance_info.enabledExtensionCount = static_cast<uint32_t>(enable_exts.size());
		instance_info.ppEnabledExtensionNames = enable_exts.data();
		instance_info.enabledLayerCount = static_cast<uint32_t>(requested_validation_layers.size());
		instance_info.ppEnabledLayerNames = requested_validation_layers.data();

#if defined(FLOWER_DEBUG)
		VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
		VkDebugReportCallbackCreateInfoEXT debug_report_create_info = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
		if(debug_utils)
		{
			debug_utils_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			debug_utils_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			debug_utils_create_info.pfnUserCallback = debug_utils_messenger_callback;
			instance_info.pNext = &debug_utils_create_info;
		}
		else
		{
			debug_report_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT|VK_DEBUG_REPORT_WARNING_BIT_EXT;
			debug_report_create_info.pfnCallback = debug_report_callback;
			instance_info.pNext = &debug_report_create_info;
		}
#endif

		// 创建vulkan实例
		auto result = vkCreateInstance(&instance_info,nullptr,&instance);
		if(result!=VK_SUCCESS)
		{
			LOG_VULKAN_FATAL("无法创建vulkan实例！");
			throw vk_exception(result,"无法创建vulkan实例！");
		}

#if defined(FLOWER_DEBUG)
		if(debug_utils)
		{
			result = create_debug_Utils_messenger_EXT(instance,&debug_utils_create_info,nullptr,&debugUtilsMessenger);
			if(result!=VK_SUCCESS)
			{
				LOG_VULKAN_FATAL("无法创建debug utils messenger！");
				throw vk_exception(result,"无法创建debug utils messenger！");
			}
		}
		else
		{
			result = create_debug_report_callback_EXT(instance,&debug_report_create_info,nullptr,&debugReportCallback);
			if(result!=VK_SUCCESS)
			{
				LOG_VULKAN_FATAL("无法创建debug report callback！");
				throw vk_exception(result,"无法创建debug report callback！");
			}
		}
#endif
	}

	void vk_instance::destroy()
	{
#if defined(FLOWER_DEBUG)
		if(debugUtilsMessenger!=VK_NULL_HANDLE)
		{
			destroy_debug_Utils_messenger_EXT(instance,debugUtilsMessenger,nullptr);
		}
		if(debugReportCallback!=VK_NULL_HANDLE)
		{
			destroy_debug_report_callback_EXT(instance,debugReportCallback,nullptr);
		}
#endif

		if(instance!=VK_NULL_HANDLE)
		{
			vkDestroyInstance(instance,nullptr);
		}
	}
}}