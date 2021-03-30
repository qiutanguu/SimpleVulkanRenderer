#pragma once
#include "vulkan/vulkan.h"
#include "vk_device.h"
#include <memory>
#include <vector>

namespace flower { namespace graphics{
	
	class vk_command_buffer
	{
	public:
		~vk_command_buffer();
		vk_command_buffer(vk_device& device): device(device) {  }
		operator VkCommandBuffer(){ return command_buffer; }
		auto& get_instance() { return command_buffer; }
	public:
		void begin(VkCommandBufferUsageFlagBits flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		void submit(VkSemaphore signal_semaphore = VK_NULL_HANDLE);
		static std::shared_ptr<vk_command_buffer> create(
			vk_device& in_device, 
			VkCommandPool command_pool, 
			VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, 
			VkQueue queue = VK_NULL_HANDLE);

		std::vector<VkPipelineStageFlags> wait_flags = {};
		std::vector<VkSemaphore> wait_semaphores = {};

	private:
		VkFence fence;
		VkCommandBuffer command_buffer;
		VkCommandPool pool;
		vk_device& device;
		bool is_begin;
		VkQueue queue;
	};

} }