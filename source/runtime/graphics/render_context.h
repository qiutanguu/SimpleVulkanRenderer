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
		// TODO: ���Ľ� 
		#pragma region ���Ľ�����
			// ubo
			std::vector<buffer> uniformBuffers;
			void createUniformBuffer();
			void updateUniformBuffer(UINT currentImage,camera& cam);

			void record_renderCommand();

			mesh mesh_data = {};

			// context ��Ⱦpass
			VkRenderPass renderPass;
			void createRenderPass();

			// ͼ�ι�����������
			VkDescriptorPool descriptorPool;
			VkDescriptorSetLayout descriptorSetLayout;
			std::vector<VkDescriptorSet> descriptorSets;
			void createDescriptorPool();
			void createDescriptorSetLayout();
			void createDescriptorSet();

			// pipeline ��Ⱦ����
			VkPipeline renderPipeline;
			VkPipelineLayout pipelineLayout;
			void createGraphicsPipeline();

			// ����
			VkImage textureImage;
			VkDeviceMemory textureImageMemory;
			VkImageView textureImageView;
			VkSampler textureSampler;
			void createTextureImage();
			void createTextureImageView();
			void createTextureSampler();

			void recreate_swapchain();
			void cleanup_swapchain();
			
			// ����buffer
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
		// ������
		swapchain vk_swapchain;

		// ͼ�ι��ߵ�Command Pool
		VkCommandPool commandPool;
		
		// FrameBuffers
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// �����
		std::vector<VkCommandBuffer> commandBuffers;

		// ͬ��
		size_t currentFrame = 0;
		const int MAX_FRAMES_IN_FLIGHT = 2; // ͬʱ�����֡��
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		// ��Ȼ���
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;
	private:
		device vk_device;
		VkSurfaceKHR surface;
		GLFWwindow* window;

	private:
		// Ϊÿ��֡���󶼷ֱ𴴽�һ��CommandBuffer
		void createCommandBuffers();
		
		// Ϊÿ������������ FrameBuffer
		void createFrameBuffers();

		// ����ͬ������
		void createSyncObjects();

		// Ϊͼ�ζ��д���Command Pool
		void createCommandPool();

		// ���������Դ
		void createDepthResources();
	};

}}