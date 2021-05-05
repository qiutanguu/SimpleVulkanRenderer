#pragma once
#include "../vk/vk_device.h"
#include "../vk/vk_buffer.h"
#include "../vk/vk_common.h"
#include <graphics/vk/vk_shader.h>

namespace flower { namespace graphics{

	class ui_overlay
	{
	private:
		bool has_init = false;
		vk_device* device;
		VkQueue queue;
		VkCommandPool pool;

		std::vector<VkPipelineShaderStageCreateInfo> shaders;

		void preparePipeline(const VkRenderPass renderPass,const VkPipelineCache pipelineCache = nullptr);
		void prepareResources();
		void freeResources();
		void initialize(vk_device* device,VkCommandPool in_pool,const VkRenderPass renderPass,const VkPipelineCache pipelineCache = nullptr);
	public:
		VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		uint32_t subpass = 0;

		std::shared_ptr<vk_buffer> vertexBuffer;
		std::shared_ptr<vk_buffer> indexBuffer;
		int32_t vertexCount = 0;
		int32_t indexCount = 0;

		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDeviceMemory fontMemory = VK_NULL_HANDLE;
		VkImage fontImage = VK_NULL_HANDLE;
		VkImageView fontView = VK_NULL_HANDLE;
		VkSampler sampler;

		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		bool visible = true;
		bool updated = false;
		float scale = 1.0f;

		ui_overlay();
		~ui_overlay();

		static std::shared_ptr<ui_overlay> create(vk_device* device,VkCommandPool in_pool,const VkRenderPass renderPass,const VkPipelineCache pipelineCache = nullptr);

		void on_swapchain_change(const VkRenderPass renderPass,const VkPipelineCache pipelineCache = nullptr);

		bool update();
		bool need_update();
		void draw(const VkCommandBuffer commandBuffer);
		void resize(uint32_t width,uint32_t height);
	};
	

} }