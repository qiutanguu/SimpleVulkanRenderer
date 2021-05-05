#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vk_common.h"

namespace flower { namespace graphics{

	class vk_device
	{
	public:
		operator VkDevice() { return device; }

		vk_device(){ }
		~vk_device(){ }

		void initialize(
			VkInstance instance,
			VkSurfaceKHR surface,
			VkPhysicalDeviceFeatures features = {},
			const std::vector<const char*>& device_request_extens= {});

		void destroy();

		// 查找队列族次序
		vk_queue_family_indices find_queue_families();

		// 内存类型查询
		uint32_t find_memory_type(uint32_t typeFilter,VkMemoryPropertyFlags properties);

		// 查询交换链支持细节
		vk_swapchain_support_details query_swapchain_support();

		// 打印所有gpu中队列族的信息
		void print_all_queue_families_info();

		// 开启的插件
		const auto& get_device_extensions() const { return device_extensions; }

		// 开启的特性
		const auto& get_device_features() const { return open_features; }
	private:
		// 选择合适的gpu
		void pick_suitable_gpu(VkInstance& instance,const std::vector<const char*>& device_request_extens);

		// 目前的gpu是否合适（支持所有的设备拓展）！
		bool is_physical_device_suitable(const std::vector<const char*>& device_request_extens);

		// 检查设备插件支持
		bool check_device_extension_support(const std::vector<const char*>& device_request_extens);

		void create_logic_device();

	public:
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkPhysicalDeviceProperties device_properties;

		VkQueue graphics_queue; // 图形队列
		VkQueue present_queue;  // 显示队列
		VkQueue compute_queue;  // 计算队列

	private:
		VkInstance instance;
		VkSurfaceKHR surface;

		// 开启的设备插件
		std::vector<const char*> device_extensions;

		// 开启的设备特性
		VkPhysicalDeviceFeatures open_features;
	};


}}