#pragma once

#include <string>

class PathUtils
{
public:
	static std::string GetModuleFolder();
	static bool FileExists(const std::string& fname);
};
