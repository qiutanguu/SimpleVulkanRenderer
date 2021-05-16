#pragma once
#include "baker.h"
#include "asset_system/asset_texture.h"

namespace baker
{
	bool bake_image(
		const std::filesystem::path& input,
		const std::filesystem::path& output,
		flower::asset::texture_format format
	);
}