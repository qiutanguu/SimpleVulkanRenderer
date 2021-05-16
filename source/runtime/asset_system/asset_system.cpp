#include "asset_system.h"
#include <fstream>
#include <iostream>

namespace flower { namespace asset{

	bool save_bin_file(const char* path,const asset_file& file)
	{
		std::ofstream outfile;
		outfile.open(path,std::ios::binary | std::ios::out);

		if(!outfile.is_open())
		{
			LOG_IO_FATAL("写入文件{0}失败！",path);
			return false;
		}

		// 1. type
		outfile.write(file.type,4);

		// 2. version
		uint32_t version = file.version;
		outfile.write((const char*)&version,sizeof(uint32_t));

		// 3. json lenght
		uint32_t lenght = static_cast<uint32_t>(file.json.size());
		outfile.write((const char*)&lenght,sizeof(uint32_t));

		// 4. blob lenght
		uint32_t bloblenght = static_cast<uint32_t>(file.bin_blob.size());
		outfile.write((const char*)&bloblenght,sizeof(uint32_t));

		// 5. json stream
		outfile.write(file.json.data(), lenght);

		// 6. blob stream
		outfile.write(file.bin_blob.data(), file.bin_blob.size());

		outfile.close();
		return true;
	}

	bool load_bin_file(const char* path,asset_file& out_file)
	{
		std::ifstream infile;
		infile.open(path,std::ios::binary);

		if(!infile.is_open())
		{
			LOG_IO_FATAL("文件{0}打开失败！",path);
			return false;
		}

		infile.seekg(0);
		infile.read(out_file.type,4);
		infile.read((char*)&out_file.version,sizeof(uint32_t));

		uint32_t jsonlen = 0;
		infile.read((char*)&jsonlen,sizeof(uint32_t));

		uint32_t bloblen = 0;
		infile.read((char*)&bloblen,sizeof(uint32_t));

		out_file.json.resize(jsonlen);

		infile.read(out_file.json.data(),jsonlen);

		out_file.bin_blob.resize(bloblen);
		infile.read(out_file.bin_blob.data(),bloblen);

		infile.close();

		return true;
	}

	asset::compress_mode parse_compress(const char* f)
	{
		if(strcmp(f,"LZ4")==0)
		{
			return asset::compress_mode::LZ4;
		}
		else
		{
			return asset::compress_mode::None;
		}
	}
} }