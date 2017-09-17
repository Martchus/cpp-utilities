#ifndef RANDOMUTILS_H
#define RANDOMUTILS_H

#include "../global.h"

#include <functional>

namespace RandomUtilities {

CPP_UTILITIES_EXPORT void generateRandomCharacterSequence(char *result, unsigned int length, bool useSmallLetters = true,
    bool useCapitalLetters = true, bool useNumbers = true, bool useSymbols = true, bool useAtLeastOneOfEachCategory = true);
CPP_UTILITIES_EXPORT void generateRandomCharacterSequence(char *result, unsigned int length, std::function<int()> randomizer, int maximalRandomNumber,
    bool useSmallLetters = true, bool useCapitalLetters = true, bool useNumbers = true, bool useSymbols = true,
    bool useAtLeastOneOfEachCategory = true);
} // namespace RandomUtilities

#endif // RANDOMUTILS_H
