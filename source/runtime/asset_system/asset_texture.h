#pragma once
#include "asset_system.h"

namespace flower { namespace asset{

	enum class texture_format : uint32_t
	{
		unknown = 0,
		R8G8B8A8_UNORM,
		R8G8B8A8_SRGB,
		R8G8B8_SRGB,
		R8G8B8_UNORM,
		R8,
	};

	struct texture_info
	{
		uint64_t texture_size;
		texture_format texture_format;
		compress_mode compression_mode;
		uint32_t pixel_size[3];
		std::string original_file;
	};

	texture_info read_texture_info(asset_file* file);
	void unpack_texture(texture_info* info,const char* sourcebuffer,size_t sourceSize,char* destination);
	asset_file pack_texture(texture_info* info,void* pixel_data);

} }