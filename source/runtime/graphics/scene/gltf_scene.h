#pragma once
#include "../vk/vk_common.h"
#include "../vk/vk_device.h"
#include "../vk/vk_vertex_buffer.h"
#include "../vk/vk_pipeline.h"
#include "vertex.h"

namespace flower{ namespace graphics{

	struct gltf_material
	{
		// pipeline ��Ⱦ����
		std::shared_ptr<vk_pipeline> pipeline_render;


		uint32_t basecolor_texture_id;

		// �õ�����������
		std::shared_ptr<vk_descriptor_set> descriptor_set;
	};
	
	struct gltf_primitive
	{
		// ��ǰprimitive�Ķ��㻺��
		std::shared_ptr<vk_index_buffer> index_buf;

		// ���ʶ�Ӧ��id
		uint32_t material_index;
	};

	class gltf_scene
	{
	public:

	public:
		// ���ж���Ķ��㻺��
		std::shared_ptr<vk_vertex_buffer> vertex_buf;

	private:
		void load_gltf(std::string file_name);

		void load_gltf_node(
			const tinygltf::Node& inputNode, 
			const tinygltf::Model& input, 
			gltf_scene::gltf_node* parent
		);


		std::string file_name;
		std::string file_folder_path;
		std::string full_file_name;
		vk_device* device;
	};


} }