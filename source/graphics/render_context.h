#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "buffer.h"
#include "swapchain.h"
#include "device.h"
#include "model.h"
#include "core/camera.h"

namespace flower { namespace graphics{

	struct uniform_buffer_object
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	
	class render_context
	{
	public:
		// TODO: 待改进 
		#pragma region 待改进部分
			// ubo
			std::vector<buffer> uniformBuffers;
			void createUniformBuffer();
			void updateUniformBuffer(UINT currentImage,camera& cam);

			void record_renderCommand();

			mesh mesh_data = {};

			// context 渲染pass
			VkRenderPass renderPass;
			void createRenderPass();

			// 图形管线描述符池
			VkDescriptorPool descriptorPool;
			VkDescriptorSetLayout descriptorSetLayout;
			std::vector<VkDescriptorSet> descriptorSets;
			void createDescriptorPool();
			void createDescriptorSetLayout();
			void createDescriptorSet();

			// pipeline 渲染管线
			VkPipeline renderPipeline;
			VkPipelineLayout pipelineLayout;
			void createGraphicsPipeline();

			// 纹理
			VkImage textureImage;
			VkDeviceMemory textureImageMemory;
			VkImageView textureImageView;
			VkSampler textureSampler;
			void createTextureImage();
			void createTextureImageView();
			void createTextureSampler();

			void recreate_swapchain();
			void cleanup_swapchain();
			
			// 顶点buffer
			buffer vertexBuffer ={};
			buffer indexBuffer = {};
			void upload_vertex_buffer();
		#pragma endregion

	public:
		render_context() : vk_swapchain({}){ }
		~render_context() { };

		void initialize(device in_device,VkSurfaceKHR in_surface,GLFWwindow* in_window);
		void destroy();


		void draw(camera& cam);

		void wait_idle();

		bool framebuffer_resized = false;
	public:
		// 交换链
		swapchain vk_swapchain;

		// 图形管线的Command Pool
		VkCommandPool commandPool;
		
		// FrameBuffers
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// 命令缓冲
		std::vector<VkCommandBuffer> commandBuffers;

		// 同步
		size_t currentFrame = 0;
		const int MAX_FRAMES_IN_FLIGHT = 2; // 同时处理的帧数
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		// 深度缓冲
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;
	private:
		device vk_device;
		VkSurfaceKHR surface;
		GLFWwindow* window;

	private:
		// 为每个帧对象都分别创建一个CommandBuffer
		void createCommandBuffers();
		
		// 为每个交换链创建 FrameBuffer
		void createFrameBuffers();

		// 创建同步对象
		void createSyncObjects();

		// 为图形队列创建Command Pool
		void createCommandPool();

		// 创建深度资源
		void createDepthResources();
	};

}}