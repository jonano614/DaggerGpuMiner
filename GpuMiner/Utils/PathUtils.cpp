#include "PathUtils.h"
#ifdef __linux__
#include <libgen.h>
#include <linux/limits.h>
#include <unistd.h>
#elif _WIN32
#include <shlwapi.h>
#endif

std::string PathUtils::GetModuleFolder()
{
#ifdef __linux__
    char result[PATH_MAX];
    if (readlink("/proc/self/exe", result, PATH_MAX) > 0) {
      return std::string(dirname(result)).append("/");
    } else {
      return "";
    }
#elif _WIN32
    char szPath[MAX_PATH];
    char szBuffer[MAX_PATH];
    char * pszFile;

    ::GetModuleFileName(NULL, (LPTCH)szPath, sizeof(szPath) / sizeof(*szPath));
    ::GetFullPathName((LPTSTR)szPath, sizeof(szBuffer) / sizeof(*szBuffer), (LPTSTR)szBuffer, (LPTSTR*)&pszFile);
    *pszFile = 0;

    return std::string(szBuffer);
#endif
}

bool PathUtils::FileExists(const std::string& fname)
{
#ifdef __linux__
    return access(fname.c_str(), F_OK) != -1;
#elif _WIN32
    return PathFileExists(fname.c_str()) == TRUE;
#endif
}
