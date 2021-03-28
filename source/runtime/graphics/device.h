#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace flower { namespace graphics{
	
	// 交换链支持细节
	struct swapchain_support_details 
	{
		// 基本表面功能（交换链中图像的最小/最大数量，图像的最小/最大宽度和高度）
		VkSurfaceCapabilitiesKHR capabilities; 

		// 表面格式（像素格式，色彩空间）
		std::vector<VkSurfaceFormatKHR> formats; 

		// 可用的演示模式
		std::vector<VkPresentModeKHR> presentModes;
	};

	// 队列族对应的次序
	class queue_family_indices 
	{
		friend class device;
	public:
		uint32_t graphics_family; // 图形队列族
		uint32_t present_family;  // 显示队列族
		uint32_t compute_faimly;  // 计算队列族

		bool isComplete() 
		{
			return graphics_family_set && present_family_set && compute_faimly_set;
		}
	private:
		bool graphics_family_set = false;
		bool present_family_set = false;
		bool compute_faimly_set = false;
	};

	class device
	{
	public:
		device(){ }
		~device(){ }
		void initialize(VkInstance instance,VkSurfaceKHR surface,VkPhysicalDeviceFeatures features = {},const std::vector<const char*>& device_request_extens= {});
		void destroy();

		// 查找队列族次序
		queue_family_indices find_queue_families();

		// 内存类型查询
		uint32_t find_memory_type(uint32_t typeFilter,VkMemoryPropertyFlags properties);

		// 查询交换链支持细节
		swapchain_support_details query_swapchain_support();

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
		VkDevice logic_device;

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