#include "PathUtils.h"
#include <shlwapi.h>

std::string PathUtils::GetModuleFolder()
{
#if _WIN32
	char szPath[MAX_PATH];
	char szBuffer[MAX_PATH];
	char * pszFile;

	::GetModuleFileName(NULL, (LPTCH)szPath, sizeof (szPath) / sizeof(*szPath));
	::GetFullPathName ((LPTSTR)szPath, sizeof (szBuffer) /  sizeof(*szBuffer), (LPTSTR)szBuffer, (LPTSTR*)&pszFile);
	*pszFile = 0;
	
	std::string ret = szBuffer;
#endif
	return ret;
}

bool PathUtils::FileExists(const std::string& fname)
{
#if _WIN32
	return PathFileExists(fname.c_str());
#endif
}
