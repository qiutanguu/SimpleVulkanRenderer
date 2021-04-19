#include "gltf_scene.h"
#include "tinygltf/tiny_gltf.h"
#include <glm/gtc/type_ptr.hpp>

namespace flower { namespace graphics{

	void gltf_scene::load_gltf_node(
		const tinygltf::Node& inputNode, 
		const tinygltf::Model& input, 
		gltf_scene::gltf_node* parent)
	{
		gltf_scene::gltf_node node{};
		node.name = inputNode.name;

		// 1. 处理当前节点模型矩阵
		node.model_matrix = glm::mat4(1.0f);
		{
			if (inputNode.translation.size() == 3) 
			{
				node.model_matrix = glm::translate(
					node.model_matrix, 
					glm::vec3(glm::make_vec3(inputNode.translation.data()))
				);
			}

			if (inputNode.rotation.size() == 4) 
			{
				glm::quat q = glm::make_quat(inputNode.rotation.data());
				node.model_matrix *= glm::mat4(q);
			}

			if (inputNode.scale.size() == 3) 
			{
				node.model_matrix = glm::scale(
					node.model_matrix, 
					glm::vec3(glm::make_vec3(inputNode.scale.data()))
				);
			}

			if (inputNode.matrix.size() == 16) 
			{
				node.model_matrix = glm::make_mat4x4(inputNode.matrix.data());
			};
		}
		
		// 2. 加载子节点
		if (inputNode.children.size() > 0) 
		{
			for (size_t i = 0; i < inputNode.children.size(); i++) 
			{
				load_gltf_node(input.nodes[inputNode.children[i]], input, &node);
			}
		}

		// 3. 如果节点含有网格数据则加载它
		if (inputNode.mesh > -1) 
		{
			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];

			// 3.1 遍历所有的primitive
			for (size_t i = 0; i < mesh.primitives.size(); i++) 
			{
				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];

				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());

				uint32_t indexCount = 0;
				// 顶点
				{
					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					const float* tangentsBuffer = nullptr;

					size_t vertexCount = 0;

					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) 
					{
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}

					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) 
					{
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// Get buffer data for vertex texture coordinates
					// glTF supports multiple sets, we only load the first one
					if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) 
					{
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// POI: This sample uses normal mapping, so we also need to load the tangents from the glTF file
					if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end()) 
					{
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						tangentsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// Append data to model's vertex buffer
					for (size_t v = 0; v < vertexCount; v++) 
					{
						Vertex vert{};
						vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
						vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
						vert.color = glm::vec3(1.0f);
						vert.tangent = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f);
						vertexBuffer.push_back(vert);
					}
				}
				// 次序
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						uint32_t* buf = new uint32_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						uint16_t* buf = new uint16_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						uint8_t* buf = new uint8_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				Primitive primitive{};
				primitive.firstIndex = firstIndex;
				primitive.indexCount = indexCount;
				primitive.materialIndex = glTFPrimitive.material;
				node.mesh.primitives.push_back(primitive);
			}
		}

		if (parent) 
		{
			parent->children.push_back(node);
		}
		else 
		{
			nodes.push_back(node);
		}
	}


	void gltf_scene::load_gltf(std::string file_name)
	{
		tinygltf::Model glTFInput;
		tinygltf::TinyGLTF gltfContext;

		std::string error, warning;
		this->device = device;

		bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, file_name);
		size_t pos = file_name.find_last_of('/');

		full_file_name = file_name;
		file_name = file_name.substr(pos);
		file_folder_path = file_name.substr(0, pos);

		std::vector<uint32_t> indexBuffer;
		std::vector<VulkanglTFScene::Vertex> vertexBuffer;

		if (fileLoaded) 
		{
			// 加载第一个场景
			const tinygltf::Scene& scene = glTFInput.scenes[0];

			for (size_t i = 0; i < scene.nodes.size(); i++) 
			{
				const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
				glTFScene.loadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
			}
		}
		else 
		{
			LOG_IO_FATAL("gltf文件{0}打开失败！", file_name);
		}

		// Create and upload vertex and index buffer
		// We will be using one single vertex buffer and one single index buffer for the whole glTF scene
		// Primitives (of the glTF model) will then index into these using index offsets

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(VulkanglTFScene::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
		glTFScene.indices.count = static_cast<uint32_t>(indexBuffer.size());

		struct StagingBuffer {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create host visible staging buffers (source)
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			vertexBuffer.data()));
		// Index data
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexBufferSize,
			&indexStaging.buffer,
			&indexStaging.memory,
			indexBuffer.data()));

		// Create device local buffers (target)
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&glTFScene.vertices.buffer,
			&glTFScene.vertices.memory));
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBufferSize,
			&glTFScene.indices.buffer,
			&glTFScene.indices.memory));

		// Copy data from staging buffers (host) do device local buffer (gpu)
		VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			vertexStaging.buffer,
			glTFScene.vertices.buffer,
			1,
			&copyRegion);

		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			indexStaging.buffer,
			glTFScene.indices.buffer,
			1,
			&copyRegion);

		vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

		// Free staging resources
		vkDestroyBuffer(device, vertexStaging.buffer, nullptr);
		vkFreeMemory(device, vertexStaging.memory, nullptr);
		vkDestroyBuffer(device, indexStaging.buffer, nullptr);
		vkFreeMemory(device, indexStaging.memory, nullptr);
	}

} }