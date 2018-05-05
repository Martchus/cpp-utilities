#ifndef CPP_UTILITIES_LEVENSHTEIN_H
#define CPP_UTILITIES_LEVENSHTEIN_H

#include "../global.h"

#include <cstring>
#include <string>

namespace MiscUtilities {

CPP_UTILITIES_EXPORT std::size_t computeDamerauLevenshteinDistance(const char *str1, std::size_t size1, const char *str2, std::size_t size2);

inline std::size_t computeDamerauLevenshteinDistance(const std::string &str1, const std::string &str2)
{
    return computeDamerauLevenshteinDistance(str1.data(), str1.size(), str2.data(), str2.size());
}

inline std::size_t computeDamerauLevenshteinDistance(const char *str1, const char *str2)
{
    return computeDamerauLevenshteinDistance(str1, std::strlen(str1), str2, std::strlen(str2));
}

} // namespace MiscUtilities

#endif // CPP_UTILITIES_LEVENSHTEIN_H
