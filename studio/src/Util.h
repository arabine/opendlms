#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <cstdint>

/*****************************************************************************/
class Util
{
public:
#ifdef USE_WINDOWS_OS
    static const char DIR_SEPARATOR = '\\';
#else
    static const char DIR_SEPARATOR = '/';
#endif

    static std::string CurrentDateTime(const std::string &format);
    static std::string ExecutablePath();
	static std::string AppDataPath();
    static std::string HomePath();
    static bool FolderExists(const std::string &foldername);
    static bool FileExists(const std::string &fileName);
    static bool Mkdir(const std::string &fullPath);
    static void ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace);
    static std::vector<std::string> Split(const std::string &theString, const std::string &delimiter);
    static std::string Join(const std::vector<std::string> &tokens, const std::string &delimiter);
    static std::int32_t GetCurrentMemoryUsage();
    static std::int32_t GetMaximumMemoryUsage();
    static std::string GetFileName(const std::string &path);
    static std::string GetDirectoryPath(const std::string &path);
    static std::int64_t FileSize(const std::string &fileName);
	static long ToLong(const std::string &value);
};

#endif // UTIL_H

//=============================================================================
// End of file Util.h
//=============================================================================
