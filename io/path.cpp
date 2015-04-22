#include "path.h"

#include "../application/global.h"
#include "../conversion/widen.h"

#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#ifdef PLATFORM_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#else
#ifdef PLATFORM_WINDOWS
#ifdef UNICODE
#undef UNICODE
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif
#include <windows.h>
#endif
#endif

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

/*!
 * Returns the file name and extension of the specified \a path string.
 */
string IoUtilities::fileName(const string &path)
{
    size_t lastSlash = path.rfind('/');
    size_t lastBackSlash = path.rfind('\\');
    size_t lastSeparator;
    if(lastSlash == string::npos && lastBackSlash == string::npos)
        return path;
    else if(lastSlash == string::npos)
        lastSeparator = lastBackSlash;
    else if(lastBackSlash == string::npos)
        lastSeparator = lastSlash;
    else
        lastSeparator = lastSlash > lastBackSlash ? lastSlash : lastBackSlash;
    return path.substr(lastSeparator + 1);
}

/*!
 * Removes invalid characters from the specified \a path string.
 *
 * The characters <, >, ?, !, *, |, /, :, \ and new lines are considered as invalid.
 */
void IoUtilities::removeInvalidChars(string &path)
{
    size_t startPos = 0;
    static const char invalidPathChars[] = {'\"', '<', '>', '?', '!', '*', '|', '/', ':', '\\', '\n'};
    for(const char *i = invalidPathChars, *end = invalidPathChars + sizeof(invalidPathChars); i != end; ++i) {
        startPos = path.find(*i);
        while(startPos != string::npos) {
            path.replace(startPos, 1, "");
            startPos = path.find(*i, startPos);
        }
    }
}

/*!
 * Locates a directory meant to store application settings.
 * \param result Specifies a string to store the path in.
 * \param applicationDirectoryName Specifies the name for the application subdirectory.
 * \param createApplicationDirectory Indicates wheter the application subdirectory should be created if not present.
 * \returns Returns if a settings directory could be located.
 */
bool IoUtilities::settingsDirectory(string &result, string applicationDirectoryName, bool createApplicationDirectory)
{
    result.clear();
    fstream pathConfigFile("path.config", ios_base::in);
    if(pathConfigFile.good()) {
        for(string line; getline(pathConfigFile, line); ) {
            string::size_type p = line.find('=');
            if((p != string::npos) && (p + 1 < line.length())) {
                string fieldName = line.substr(0, p);
                if(fieldName == "settings") {
                    result.assign(line.substr(p + 1));
                }
            }
        }
    }
    if(!result.empty()) {
#ifdef PLATFORM_UNIX
        struct stat sb;
        return (stat(result.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
#else
#ifdef PLATFORM_WINDOWS
        DWORD ftyp = GetFileAttributesA(result.c_str());
        return (ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY);
#else
#error Platform not supported.
#endif
#endif
    } else {
        if(!applicationDirectoryName.empty()) {
            removeInvalidChars(applicationDirectoryName);
        }
#ifdef PLATFORM_UNIX
        if(char *homeDir = getenv("HOME")) {
            result = string(homeDir);
        } else {
            struct passwd *pw = getpwuid(getuid());
            result = string(pw->pw_dir);
        }
        struct stat sb;
        result += "/.config";
        if(createApplicationDirectory && !(stat(result.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))) {
            if(mkdir(result.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                return false;
            }
        }
        if(!applicationDirectoryName.empty()) {
            result += "/" + applicationDirectoryName;
            if(createApplicationDirectory && !(stat(result.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))) {
                if(mkdir(result.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                    return false;
                }
            }
        }
#else
#ifdef PLATFORM_WINDOWS
        if(char *appData = getenv("appdata")) {
            result = appData;
            if(!applicationDirectoryName.empty()) {
                result += "\\" + applicationDirectoryName;
                if(createApplicationDirectory) {
                    DWORD ftyp = GetFileAttributesA(result.c_str());
                    if(ftyp == INVALID_FILE_ATTRIBUTES) {
                        return false;
                    } else if(ftyp & FILE_ATTRIBUTE_DIRECTORY) {
                        return true;
                    } else {
                        if(CreateDirectory(result.c_str(), NULL) == 0) {
                            return false;
                        } else {
                            return true;
                        }
                    }
                }
            }
        } else {
            return false;
        }
    #else
    #error Platform not supported.
    #endif
    #endif
    }
    return true;
}
