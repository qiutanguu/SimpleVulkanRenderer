#include "asset_texture.h"
#include "../core/core.h"
#include "asset_system.h"
#include <nlohmann/json.hpp>
#include <lz4/lz4.h>

namespace flower { namespace asset{

	std::string format_to_string(texture_format format)
	{
		switch(format)
		{
		case flower::asset::texture_format::unknown:
			return "unknown";
			break;
		case flower::asset::texture_format::R8G8B8A8_UNORM:
			return "R8G8B8A8_UNORM";
			break;
		case flower::asset::texture_format::R8G8B8A8_SRGB:
			return "R8G8B8A8_SRGB";
			break;
		case flower::asset::texture_format::R8G8B8_SRGB:
			return "R8G8B8_SRGB";
			break;
		case flower::asset::texture_format::R8G8B8_UNORM:
			return "R8G8B8_UNORM";
			break;
		case flower::asset::texture_format::R8:
			return "R8";
			break;
		default:
			LOG_IO_FATAL("未知的纹理格式！");
			return "ERROR";
			break;
		}

		return "ERROR";
	}
	
	texture_format parse_format(const char* f)
	{
		if(strcmp(f,"R8G8B8A8_UNORM")==0)
		{
			return texture_format::R8G8B8A8_UNORM;
		}
		else if(strcmp(f,"R8G8B8A8_SRGB")==0)
		{
			return texture_format::R8G8B8A8_SRGB;
		}
		else if(strcmp(f,"R8G8B8_SRGB")==0)
		{
			return texture_format::R8G8B8_SRGB;
		}
		else if(strcmp(f,"R8G8B8_UNORM")==0)
		{
			return texture_format::R8G8B8_UNORM;
		}
		else if(strcmp(f,"R8")==0)
		{
			return texture_format::R8;
		}
		else
		{
			return texture_format::unknown;
		}
	}

	asset_file pack_texture(texture_info* info,void* pixel_data)
	{
		// 填入Magic头
		asset_file file;
		file.type[0] = 'T';
		file.type[1] = 'E';
		file.type[2] = 'X';
		file.type[3] = 'I';
		file.version = 1;

		// meta data 填入
		nlohmann::json texture_metadata;
		texture_metadata["format"] = format_to_string(info->texture_format);
		texture_metadata["width"] = info->pixel_size[0];
		texture_metadata["height"] = info->pixel_size[1];
		texture_metadata["depth"] = info->pixel_size[2];
		texture_metadata["buffer_size"] = info->texture_size;
		texture_metadata["original_file"] = info->original_file;

		//find the maximum data needed for the compression
		auto compress_staging = LZ4_compressBound((int)info->texture_size);

		file.bin_blob.resize(compress_staging);

		auto compressed_size = LZ4_compress_default((const char*)pixel_data,file.bin_blob.data(),(int)info->texture_size,compress_staging);

		file.bin_blob.resize(compressed_size);
		texture_metadata["compression"] = "LZ4";

		std::string stringified = texture_metadata.dump();
		file.json = stringified;

		return file;
	}

	texture_info read_texture_info(asset_file* file)
	{
		texture_info info;

		nlohmann::json texture_metadata = nlohmann::json::parse(file->json);

		std::string format_string = texture_metadata["format"];
		info.texture_format = parse_format(format_string.c_str());

		std::string compression_string = texture_metadata["compression"];
		info.compression_mode = parse_compress(compression_string.c_str());

		info.texture_size = texture_metadata["buffer_size"];
		info.original_file = texture_metadata["original_file"];

		info.pixel_size[0] = texture_metadata["width"];
		info.pixel_size[1] = texture_metadata["height"];
		info.pixel_size[2] = texture_metadata["depth"];

		return info;

	}

	void unpack_texture(texture_info* info,const char* sourcebuffer,size_t sourceSize,char* destination)
	{
		if(info->compression_mode == compress_mode::LZ4)
		{
			LZ4_decompress_safe(sourcebuffer, destination, (int)sourceSize, (int)info->texture_size);
		}
		else
		{
			memcpy(destination,sourcebuffer,sourceSize);
		}
	}
} }