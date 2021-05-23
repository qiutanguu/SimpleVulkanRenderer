#include "asset_system/asset_system.h"
#include "asset_system/asset_texture.h"
#include "baker.h"
#include "bake_texture.h"
#include "core/core.h"

int main(int argc,char** argvs)
{
	std::filesystem::path path { argvs[1]};

	std::filesystem::path directory = path;

	LOG_IO_INFO("加载资产文件夹{0}。",directory.c_str());

	for (auto& p : std::filesystem::directory_iterator(directory))
	{
		LOG_IO_TRACE("加载文件{0}。",p.path().c_str());

		if (p.path().extension() == ".png" || p.path().extension() == ".tga") 
		{
			LOG_IO_TRACE("发现纹理{0}。",p.path().c_str());

			auto newpath = p.path();
			/*
			newpath.replace_extension(".flower");

			std::string new_path_str = newpath.c_str();
			std::string::size_type idx = str.find( str1 );
			if(newpath)

			bake_image(p.path(), newpath,);
			*/
		}
	}

	return 0;
}