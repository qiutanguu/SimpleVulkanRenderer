#include <stb/stb_image.h>
#include "bake_texture.h"
#include "asset_system/asset_texture.h"
#include "asset_system/asset_system.h"
#include "core/core.h"

namespace baker
{
	bool bake_image(
		const std::filesystem::path& input,
		const std::filesystem::path& output,
		flower::asset::texture_format format
	)
	{
		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) 
		{
			LOG_IO_ERROR("º”‘ÿÕº∆¨{0} ß∞‹£°",input);
			return false;
		}

		int texture_size = texWidth * texHeight * 4;

		flower::asset::texture_info texinfo;
		texinfo.texture_size = texture_size;
		texinfo.pixel_size[0] = texWidth;
		texinfo.pixel_size[1] = texHeight;
		texinfo.pixel_size[2] = 1;
		texinfo.texture_format = format;	
		texinfo.original_file = input.string();
		flower::asset::asset_file newImage = flower::asset::pack_texture(&texinfo, pixels);	

		stbi_image_free(pixels);

		flower::asset::save_bin_file(output.string().c_str(), newImage);
		return true;
	}
}