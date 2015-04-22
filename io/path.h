#ifndef PATHHELPER_H
#define PATHHELPER_H

#include "binarywriter.h"
#include "binaryreader.h"

#include "../application/global.h"

#include <string>

namespace IoUtilities
{

LIB_EXPORT std::string fileName(const std::string &path);
LIB_EXPORT void removeInvalidChars(std::string &path);
LIB_EXPORT bool settingsDirectory(std::string &result, std::string applicationDirectoryName = std::string(), bool createApplicationDirectory = false);

}

#endif // PATHHELPER_H
