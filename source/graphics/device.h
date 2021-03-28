#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace flower { namespace graphics{
	
	// ������֧��ϸ��
	struct swapchain_support_details 
	{
		// �������湦�ܣ���������ͼ�����С/���������ͼ�����С/����Ⱥ͸߶ȣ�
		VkSurfaceCapabilitiesKHR capabilities; 

		// �����ʽ�����ظ�ʽ��ɫ�ʿռ䣩
		std::vector<VkSurfaceFormatKHR> formats; 

		// ���õ���ʾģʽ
		std::vector<VkPresentModeKHR> presentModes;
	};

	// �������Ӧ�Ĵ���
	class queue_family_indices 
	{
		friend class device;
	public:
		uint32_t graphics_family; // ͼ�ζ�����
		uint32_t present_family;  // ��ʾ������
		uint32_t compute_faimly;  // ���������

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

		// ���Ҷ��������
		queue_family_indices find_queue_families();

		// �ڴ����Ͳ�ѯ
		uint32_t find_memory_type(uint32_t typeFilter,VkMemoryPropertyFlags properties);

		// ��ѯ������֧��ϸ��
		swapchain_support_details query_swapchain_support();

		// ��ӡ����gpu�ж��������Ϣ
		void print_all_queue_families_info();

		// �����Ĳ��
		const auto& get_device_extensions() const { return device_extensions; }

		// ����������
		const auto& get_device_features() const { return open_features; }
	private:
		// ѡ����ʵ�gpu
		void pick_suitable_gpu(VkInstance& instance,const std::vector<const char*>& device_request_extens);

		// Ŀǰ��gpu�Ƿ���ʣ�֧�����е��豸��չ����
		bool is_physical_device_suitable(const std::vector<const char*>& device_request_extens);

		// ����豸���֧��
		bool check_device_extension_support(const std::vector<const char*>& device_request_extens);

		void create_logic_device();

	public:
		VkPhysicalDevice physical_device;
		VkDevice logic_device;

		VkQueue graphics_queue; // ͼ�ζ���
		VkQueue present_queue;  // ��ʾ����
		VkQueue compute_queue;  // �������

	private:
		VkInstance instance;
		VkSurfaceKHR surface;

		// �������豸���
		std::vector<const char*> device_extensions;

		// �������豸����
		VkPhysicalDeviceFeatures open_features;
	};


}}