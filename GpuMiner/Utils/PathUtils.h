// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#include <string>

class PathUtils
{
public:
	static std::string GetModuleFolder();
	static bool FileExists(const std::string& fname);
};
