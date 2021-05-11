#pragma once
#include "../core/core.h"

namespace flower { namespace asset{

	struct asset_file
	{
		char type[4];
		int version;
		std::string json;
		std::vector<char> bin_blob;
	};

	enum class compress_mode : uint32_t 
	{
		None,
		LZ4
	};

	bool save_bin_file(const char* path,const asset_file& file);
	bool load_bin_file(const char* path,asset_file& out_file);
	asset::compress_mode parse_compress(const char* f);
} }