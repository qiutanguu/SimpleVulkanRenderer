#pragma once

#include "graphics/vk_runtime.h"
#include "graphics/vk/vk_buffer.h"
#include "graphics/scene/mesh.h"
#include "graphics/vk/vk_vertex_buffer.h"
#include "graphics/vk/vk_pipeline.h"
#include "graphics/vk/vk_texture.h"
#include "graphics/vk/vk_shader.h"
#include "graphics/vk/vk_renderpass.h"
#include "graphics/vk/vk_device.h"
#include "graphics/vk/vk_command_buffer.h"
#include "graphics/scene_textures.h"

namespace flower{ namespace graphics{

	struct cluster_aabb
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct cluster_dim
	{
		// 相机fov_y的一半
		float fov_y;

		// 相机znear和zfar
		float z_near;
		float z_far;

		float sD;
		float log_dim_y;
		float log_depth;

		int32_t cluster_dim_x;
		int32_t cluster_dim_y;
		int32_t cluster_dim_z;

		// cluster_dim_x * cluster_dim_y * cluster_dim_z
		int32_t cluster_dim_xyz;

		// 当相机fovy改变或窗口大小改变时需要重建
		static cluster_dim build(const camera& cam,int32_t block_size)
		{
			cluster_dim ret{};

			ret.fov_y = glm::radians(cam.Zoom)*0.5f;
			ret.z_near = cam.zNear;
			ret.z_far = cam.zFar;

			auto extent = g_scene_textures.scene_color->get_extent_2d();
			ret.cluster_dim_x = int32_t(glm::ceil(extent.width/float(block_size)));
			ret.cluster_dim_y = int32_t(glm::ceil(extent.height/float(block_size)));

			ret.sD = 2.0f * tan(ret.fov_y) / float(ret.cluster_dim_y);
			ret.log_dim_y = 1.0f / log(1.0f + ret.sD);
			ret.log_depth = log(ret.z_far / ret.z_near);
			ret.cluster_dim_z = int32_t(glm::floor(ret.log_depth * ret.log_dim_y));

			ret.cluster_dim_xyz = ret.cluster_dim_x * ret.cluster_dim_y * ret.cluster_dim_z;
			return ret;
		}
	};

	class compute_deferred_cluster
	{
	private:
		vk_device* device;
		cluster_dim cluster_dim_info;


	public:
		void on_swapchain_recreate();
	};

}}