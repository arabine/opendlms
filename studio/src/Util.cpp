
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "Util.h"

#ifdef USE_WINDOWS_OS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Shlobj.h>  // need to include definitions of constants
#include <direct.h>
#include <sys/stat.h>
#include <windows.h>
#include <psapi.h>
#include <string>
#include <cwchar>
#include <cassert>

#pragma warning(disable: 4996) // allow unsafe functions, consider change the code in the future

template<typename Td>
Td string_cast(const wchar_t* pSource, unsigned int codePage = CP_ACP);


template<>
std::string string_cast(const wchar_t* pSource, unsigned int codePage)
{
	assert(pSource != 0);
	size_t sourceLength = std::wcslen(pSource);
	if (sourceLength > 0)
	{
		int length = ::WideCharToMultiByte(codePage, 0, pSource, sourceLength, NULL, 0, NULL, NULL);
		if (length == 0)
			return std::string();

		std::vector<char> buffer(length);
		::WideCharToMultiByte(codePage, 0, pSource, sourceLength, &buffer[0], length, NULL, NULL);

		return std::string(buffer.begin(), buffer.end());
	}
	else
		return std::string();

}

#endif

#ifdef USE_UNIX_OS
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/resource.h>
#endif

#ifdef USE_APPLE_OS
#include <mach/mach.h>
#endif

/*****************************************************************************/
/**
 * @brief Util::CurrentDateTime
 *
 * Gets current date/time, format is given in parameters
 * Example: "%Y-%m-%d.%X" generates the following output: 2014-03-21.18:20:49
 *
 * @param format
 * @return
 */
std::string Util::CurrentDateTime(const std::string &format)
{
    std::stringstream datetime;

    time_t rawtime;
    struct tm *timeinfo;
    char buffer [80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), format.c_str(), timeinfo);

    datetime << buffer;
    /*
     * This code is the C++0x11 way of formating date, but GCC does not support it yet :(
        std::stringstream datetime;
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        datetime << std::put_time(&tm, format);
    */
    return datetime.str();
}
/*****************************************************************************/
std::string Util::ExecutablePath()
{
    std::string path;

#ifdef USE_WINDOWS_OS
    wchar_t buf[MAX_PATH];

    // Will contain exe path
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
        // When passing NULL to GetModuleHandle, it returns handle of exe itself
        GetModuleFileName(hModule, buf, MAX_PATH);
    }
    std::wstring wstr(buf);
    path = std::string(wstr.begin(), wstr.end());

#elif defined(USE_UNIX_OS)
    char buf[FILENAME_MAX];
    readlink("/proc/self/exe", buf, sizeof(buf));
    path = buf;
#elif defined(USE_APPLE_OS)
    _NSGetExecutablePath(path, &size); // make it compile
#else
#error "A portable code is needed here"
#endif
    return (GetDirectoryPath(path));
}
/*****************************************************************************/
std::string Util::AppDataPath()
{
	std::string appdir;

#if defined(USE_WINDOWS_OS)
	WCHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path)))
	{
		appdir = string_cast<std::string>(path) + "AppData\\Roaming";
	}

#else	
	std::cout << "AppDataPath() not implemented" << std::endl;
#endif
	return appdir;
}
/*****************************************************************************/
std::string Util::HomePath()
{
    std::string homedir;

#if defined(USE_WINDOWS_OS)
	std::cout << "HomePath() not implemented" << std::endl;
#if defined(_MSC_VER)
	char* buf = 0;
	size_t sz = 0;
	if (_dupenv_s(&buf, &sz, "HOMEDRIVE") == 0)
	{
		homedir.append(buf);
		free(buf);
	}
	if (_dupenv_s(&buf, &sz, "HOMEPATH") == 0)
	{
		homedir.append(buf);
		free(buf);
	}
#else
    homedir = getenv("HOMEDRIVE");
    homedir += getenv("HOMEPATH");
#endif
#else
    homedir = getenv("HOME");
#endif
    return homedir;
}
/*****************************************************************************/
/**
 * Checks if a folder exists
 * @param foldername path to the folder to check.
 * @return true if the folder exists, false otherwise.
 */
bool Util::FolderExists(const std::string &foldername)
{
    bool ret = false;
    struct stat st;
    if (::stat(foldername.c_str(), &st) == 0)
    {
        if ((st.st_mode & S_IFDIR) != 0)
        {
            ret = true;
        }
    }
    return ret;
}
/*****************************************************************************/
/**
 * Checks if a file exists
 * @param filename of the file to check.
 * @return true if the file exists, false otherwise.
 */
bool Util::FileExists(const std::string &fileName)
{
    bool ret = false;
    struct stat st;
    if (::stat(fileName.c_str(), &st) == 0)
    {
        if ((st.st_mode & S_IFREG) != 0)
        {
            ret = true;
        }
    }
    return ret;
}
/*****************************************************************************/
/**
 * @brief Util::FileSize
 * @param fileName
 * @return -1 means file not found
 */
std::int64_t Util::FileSize(const std::string &fileName)
{
    std::int64_t size = -1;
    struct stat st;
    if (::stat(fileName.c_str(), &st) == 0)
    {
        size = st.st_size;
    }
    return size;
}
/*****************************************************************************/
std::string Util::GetFileName(const std::string &path)
{
    return path.substr( path.find_last_of(DIR_SEPARATOR) + 1 );
}
/*****************************************************************************/
std::string Util::GetDirectoryPath(const std::string &path)
{
    std::string directory = path;
    // transform into native path
#ifdef USE_WINDOWS_OS
    ReplaceCharacter(directory, "/", "\\");
#endif

    return directory.substr(0, directory.find_last_of(DIR_SEPARATOR));
}
/*****************************************************************************/
/**
 * Portable wrapper for mkdir. Internally used by mkdir()
 * @param[in] path the full path of the directory to create.
 * @return zero on success, otherwise -1.
 */
int my_mkdir(const char *path)
{
#if defined(USE_WINDOWS_OS)
    return ::_mkdir(path);
#else
    return ::mkdir(path, 0755); // not sure if this works on mac
#endif
}
/*****************************************************************************/
/**
 * Recursive, portable wrapper for mkdir.
 * @param[in] path the full path of the directory to create.
 * @return zero on success, otherwise -1.
 */
bool Util::Mkdir(const std::string &fullPath)
{
    bool ret = true;
    std::string temp = fullPath;

#ifdef USE_WINDOWS_OS
    ReplaceCharacter(temp, "/", "\\");
#endif

    std::string current_level = "";
    std::string level;
    std::stringstream ss(temp);

    // split path using slash as a separator
    while (std::getline(ss, level, Util::DIR_SEPARATOR))
    {
        current_level += level; // append folder to the current level

        if (!FolderExists(current_level))
        {
#ifdef USE_WINDOWS_OS
            if ((current_level.size() == 2) &&
                    (current_level[1] == ':'))
            {
                // Do nothing, skip disk name
            }
            else
#endif
                if (current_level.size() > 2)
                {
                    // create current level
                    if (my_mkdir(current_level.c_str()) != 0)
                    {
                        ret = false;
                        break;
                    }
                }
        }
        current_level += Util::DIR_SEPARATOR; // don't forget to append a slash
    }

    return ret;
}
/*****************************************************************************/
void Util::ReplaceCharacter(std::string &theString, const std::string &toFind, const std::string &toReplace)
{
    std::size_t found;
    do
    {
        found = theString.find(toFind);
        if (found != std::string::npos)
        {
            theString.replace(found, 1, toReplace);
        }
    }
    while (found != std::string::npos);
}
/*****************************************************************************/
std::vector<std::string> Util::Split(const std::string &theString, const std::string &delimiter)
{
    std::vector<std::string> theStringVector;
    size_t  start = 0, end = 0;

    while (end != std::string::npos)
    {
        end = theString.find(delimiter, start);

        // If at end, use length=maxLength.  Else use length=end-start.
        theStringVector.push_back(theString.substr(start,
                                  (end == std::string::npos) ? std::string::npos : end - start));

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = ((end > (std::string::npos - delimiter.size()))
                 ?  std::string::npos  :  end + delimiter.size());
    }
    return theStringVector;
}
/*****************************************************************************/
std::string Util::Join(const std::vector<std::string> &tokens, const std::string &delimiter)
{
    std::stringstream ss;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        // Add the delimiter between tokens only, nothing at the end of the string
        if (i != 0)
        {
            ss << delimiter;
        }
        ss << tokens[i];
    }

    return ss.str();
}
/*****************************************************************************/
/**
 * @brief Util::GetCurrentMemoryUsage
 *
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 *
 */
std::int32_t Util::GetCurrentMemoryUsage()
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
        return (size_t)0L;      /* Can't access? */
    return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return (size_t)0L;          /* Unsupported. */
#endif
}
/*****************************************************************************/
/**
 * @brief GetMaximumMemoryUsage
 *
 * Returns the peak (maximum so far) resident set size (physical memory use)
 * measured in bytes, or zero if the value cannot be determined on this OS.
 */
std::int32_t Util::GetMaximumMemoryUsage()
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    /* AIX and Solaris ------------------------------------------ */
    struct psinfo psinfo;
    int fd = -1;
    if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
        return (size_t)0L;      /* Can't open? */
    if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
    {
        close( fd );
        return (size_t)0L;      /* Can't read? */
    }
    close( fd );
    return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* BSD, Linux, and OSX -------------------------------------- */
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
    /* Unknown OS ----------------------------------------------- */
    return (size_t)0L;          /* Unsupported. */
#endif
}
/*****************************************************************************/
long Util::ToLong(const std::string &value)
{
	const char* start = value.c_str();
	char* end;
	// This parses "1234" (decimal) and also "0x4D2" (hex)
	long n = strtol(start, &end, 0);
	return end > start ? n : 0;
}

//=============================================================================
// End of file Util.cpp
//=============================================================================
