#pragma once
#include <fstream>
#include <filesystem>

struct convert_state
{
	std::filesystem::path asset_path;
	std::filesystem::path export_path;

	std::filesystem::path convert_to_export_relative(std::filesystem::path path) const
	{
		return path.lexically_proximate(export_path);
	}
};