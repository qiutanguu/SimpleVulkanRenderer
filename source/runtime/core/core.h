#pragma once

#if defined(_DEBUG) || defined(DEBUG)
// 调试宏
#define FLOWER_DEBUG
#endif

// 开启 Log 文本输出
// #define LOG_ENABLE_FILE

// Log 打印线程ID
#define LOG_THREAD_ID

// 开启Log打印
#define ENABLE_LOG

#include "log.h"

#ifdef ENABLE_LOG
#define LOG_TRACE(...)    ::flower::logger::get_instance()->get_logger_util()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::flower::logger::get_instance()->get_logger_util()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::flower::logger::get_instance()->get_logger_util()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::flower::logger::get_instance()->get_logger_util()->error(__VA_ARGS__)
#define LOG_FATAL(...)    ::flower::logger::get_instance()->get_logger_util()->critical(__VA_ARGS__); throw std::runtime_error("utils fatal!")

#define LOG_IO_TRACE(...)    ::flower::logger::get_instance()->get_logger_io()->trace(__VA_ARGS__)
#define LOG_IO_INFO(...)     ::flower::logger::get_instance()->get_logger_io()->info(__VA_ARGS__)
#define LOG_IO_WARN(...)     ::flower::logger::get_instance()->get_logger_io()->warn(__VA_ARGS__)
#define LOG_IO_ERROR(...)    ::flower::logger::get_instance()->get_logger_io()->error(__VA_ARGS__)
#define LOG_IO_FATAL(...)    ::flower::logger::get_instance()->get_logger_io()->critical(__VA_ARGS__); throw std::runtime_error("io fatal!")

#define LOG_VULKAN_TRACE(...)    ::flower::logger::get_instance()->get_logger_graphics()->trace(__VA_ARGS__)
#define LOG_VULKAN_INFO(...)     ::flower::logger::get_instance()->get_logger_graphics()->info(__VA_ARGS__)
#define LOG_VULKAN_WARN(...)     ::flower::logger::get_instance()->get_logger_graphics()->warn(__VA_ARGS__)
#define LOG_VULKAN_ERROR(...)    ::flower::logger::get_instance()->get_logger_graphics()->error(__VA_ARGS__)
#define LOG_VULKAN_FATAL(...)    ::flower::logger::get_instance()->get_logger_graphics()->critical(__VA_ARGS__); throw std::runtime_error("graphics fatal!")
#else
#define LOG_TRACE(...)   
#define LOG_INFO(...)    
#define LOG_WARN(...)   
#define LOG_ERROR(...)    
#define LOG_FATAL(...)   throw std::runtime_error("utils fatal!")

#define LOG_IO_TRACE(...) 
#define LOG_IO_INFO(...)
#define LOG_IO_WARN(...)    
#define LOG_IO_ERROR(...)   
#define LOG_IO_FATAL(...)  throw std::runtime_error("utils fatal!")

#define LOG_VULKAN_TRACE(...) 
#define LOG_VULKAN_INFO(...)
#define LOG_VULKAN_WARN(...)  
#define LOG_VULKAN_ERROR(...)   
#define LOG_VULKAN_FATAL(...)  throw std::runtime_error("utils fatal!")
#endif

#if defined(FLOWER_DEBUG)
#define ASSERT(x, ...) { if(!(x)) { LOG_FATAL("断言失败: {0}！", __VA_ARGS__); __debugbreak(); } }
#define CHECK(x) { if(!(x)) { LOG_FATAL("检查出错！"); __debugbreak(); } }
#else
#define ASSERT(x, ...)  { if(!(x)) { LOG_FATAL("断言失败: {0}！", __VA_ARGS__); } }
#define CHECK(x)  { if(!(x)) { LOG_FATAL("检查出错！"); } }
#endif

#include <sstream>
#include <string>
#include <fstream>
#include <array>

// Vulkan glm相关配置
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <tinyobjloader/tiny_obj_loader.h>

namespace flower
{
	struct non_copyable
	{
		non_copyable() = default;
		non_copyable(const non_copyable&) = delete;
		non_copyable& operator=(const non_copyable&) = delete;
	};

	inline std::vector<char> read_file_binary(const std::string& filename)
	{
		std::ifstream file(filename,std::ios::ate|std::ios::binary);

		if(!file.is_open())
		{
			LOG_IO_FATAL("文件{0}打开失败",filename);
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(),fileSize);
		file.close();
		return buffer;
	}
}