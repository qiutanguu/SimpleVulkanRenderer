#pragma once
#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "core/camera.h"

namespace flower{ namespace graphics{
	
	class viking_room_scene : public vk_runtime
	{
	public:
		viking_room_scene(camera& incam,GLFWwindow* window) : 
			vk_runtime::vk_runtime(window),
			scene_view_cam(incam)
		{
		
		}

	public:
		virtual void config_before_init() override;
		virtual void initialize_special() override;
		virtual void tick(float time, float delta_time) override;
		virtual void destroy_special() override;
		virtual void recreate_swapchain() override;
		virtual void cleanup_swapchain() override;
		

	private:
		std::vector<std::shared_ptr<vk_buffer>> uniformBuffers;
		void create_uniform_buffer();

		void update_before_commit(uint32_t backBuffer_index);
		void record_renderCommand();

		mesh mesh_data = {};

		// 图形管线描述符池
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSet> descriptorSets;
		void createDescriptorPool();
		void createDescriptorSetLayout();
		void createDescriptorSet();

		// pipeline 渲染管线
		VkPipelineLayout pipeline_layout;
		VkPipeline render_pipeline;
		void createGraphicsPipeline();

		// 纹理
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();

		// 顶点buffer
		std::shared_ptr<vk_vertex_buffer> vertex_buffer;
		std::shared_ptr<vk_index_buffer> index_buffer;
		void upload_vertex_buffer();

		camera& scene_view_cam;
	};

} }