#include "vk_device.h"
#include "vk_common.h"
#include <set>

namespace flower{ namespace graphics{

	void vk_device::initialize(VkInstance instance,VkSurfaceKHR surface,VkPhysicalDeviceFeatures features,const std::vector<const char*>& device_request_extens)
	{
		this->instance = instance;
		this->surface = surface;

		// 1. 选择合适的Gpu
		pick_suitable_gpu(this->instance,device_request_extens);

		// 2. 创建逻辑设备
		this->device_extensions = device_request_extens;
		this->open_features = features;
		create_logic_device();
	}

	void vk_device::destroy()
	{
		if (device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(device, nullptr);
		}
	}

	void vk_device::create_logic_device()
	{
		// 1. 指定要创建的队列
		auto indices = find_queue_families();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphics_family, indices.present_family,indices.compute_faimly };

		// 创建graphicsFamily presentFamily computeFaimly对应队列 
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) 
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// 2. 开始填写创建结构体信息
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data(); // 创建多个队列
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &open_features;

		// 3. 开启设备需要的扩展
		createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		createInfo.ppEnabledExtensionNames = device_extensions.data();

		// 没有特定于设备的层，所有的层都是来自实例控制
		createInfo.ppEnabledLayerNames = NULL;
		createInfo.enabledLayerCount = 0;

		if (vkCreateDevice(physical_device, &createInfo, nullptr, &device) != VK_SUCCESS) 
		{
			LOG_VULKAN_FATAL("创建逻辑设备失败！");
		}

		// 获取队列
		vkGetDeviceQueue(device,indices.graphics_family,0,&graphics_queue);
		vkGetDeviceQueue(device,indices.present_family,0,&present_queue);
		vkGetDeviceQueue(device,indices.compute_faimly,0,&compute_queue);
	}

	void vk_device::pick_suitable_gpu(VkInstance& instance,const std::vector<const char*>& device_request_extens)
	{
		// 1. 查询所有的Gpu
		uint32_t physical_device_count{0};
		vk_check(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
		if (physical_device_count < 1)
		{
			LOG_VULKAN_FATAL("找不到支持Vulkan的Gpu！");
		}
		std::vector<VkPhysicalDevice> physical_devices;
		physical_devices.resize(physical_device_count);
		vk_check(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));

		// 2. 选择第一个独立Gpu
		ASSERT(!physical_devices.empty(),"本机上找不到Gpu！");

		// 找第一个独立Gpu
		for (auto &gpu : physical_devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				this->physical_device = gpu;
				if (is_physical_device_suitable(device_request_extens)) 
				{
					LOG_VULKAN_INFO("选择独立显卡: {0}",vulkan_to_string(deviceProperties.deviceName));
					return;
				}
			}
		}

		// 否则直接返回第一个gpu
		LOG_VULKAN_WARN("找不到独立显卡，将使用默认显卡渲染！");

		this->physical_device = physical_devices.at(0);
		if (is_physical_device_suitable(device_request_extens)) 
		{
			this->physical_device = physical_devices.at(0);
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(this->physical_device,&deviceProperties);
			LOG_VULKAN_INFO("选择默认显卡: {0}",vulkan_to_string(deviceProperties.deviceName));
			return;
		}
		else
		{
			LOG_VULKAN_FATAL("没有合适的Gpu可以使用！");
		}
	}


	bool vk_device::is_physical_device_suitable(const std::vector<const char*>& device_request_extens)
	{
		// 满足所有的队列族
		vk_queue_family_indices indices = find_queue_families();

		// 支持所有的设备插件
		bool extensionsSupported = check_device_extension_support(device_request_extens);

		bool swapChainAdequate = false;
		// 支持交换链格式不为空
		if (extensionsSupported) 
		{
			auto swapChainSupport = query_swapchain_support();
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.is_completed() && extensionsSupported && swapChainAdequate;
	}

	vk_queue_family_indices vk_device::find_queue_families()
	{
		vk_queue_family_indices indices;

		// 1. 找到所有的队列族
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &queueFamilyCount, queueFamilies.data());

		// 2. 找到支持需求的队列族
		int i = 0;
		for (const auto& queueFamily : queueFamilies) 
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.graphics_family = i;
				indices.graphics_family_set = true;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 
			{
				indices.compute_faimly = i;
				indices.compute_faimly_set = true;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport);
			if (presentSupport) 
			{
				indices.present_family = i;
				indices.present_family_set = true;
			}

			if (indices.is_completed()) {
				break;
			}

			i++;
		}

		return indices;
	}

	vk_swapchain_support_details vk_device::query_swapchain_support()
	{
		// 查询基本表面功能
		vk_swapchain_support_details details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

		// 查询表面格式
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,surface, &formatCount, nullptr);
		if (formatCount != 0) 
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
		}

		// 3. 查询展示格式
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) 
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	uint32_t vk_device::find_memory_type(uint32_t typeFilter,VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}

		LOG_VULKAN_FATAL("找不到合适的内存类型！");
	}

	void vk_device::print_all_queue_families_info()
	{
		// 1. 找到所有的队列族
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

		for(auto &q_family : queueFamilies)
		{
			LOG_VULKAN_INFO("队列id: {}",vulkan_to_string(q_family.queueCount));
			LOG_VULKAN_INFO("队列标志: {}",vulkan_to_string(q_family.queueFlags));
		}
	}

	bool vk_device::check_device_extension_support(const std::vector<const char*>& device_request_extens)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(device_request_extens.begin(), device_request_extens.end());

		for (const auto& extension : availableExtensions) 
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
}}