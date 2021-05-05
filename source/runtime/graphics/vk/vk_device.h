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

		// ���Ҷ��������
		vk_queue_family_indices find_queue_families();

		// �ڴ����Ͳ�ѯ
		uint32_t find_memory_type(uint32_t typeFilter,VkMemoryPropertyFlags properties);

		// ��ѯ������֧��ϸ��
		vk_swapchain_support_details query_swapchain_support();

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
		VkDevice device;
		VkPhysicalDeviceProperties device_properties;

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