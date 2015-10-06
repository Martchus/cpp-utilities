#ifndef RANDOMUTILS_H
#define RANDOMUTILS_H

#include "../application/global.h"

#include <functional>

namespace RandomUtilities {

LIB_EXPORT void generateRandomCharacterSequence(char *result, unsigned int length, bool useSmallLetters = true, bool useCapitalLetters = true, bool useNumbers = true, bool useSymbols = true, bool useAtLeastOneOfEachCategory = true);
LIB_EXPORT void generateRandomCharacterSequence(char *result, unsigned int length, std::function<int ()> randomizer, int maximalRandomNumber, bool useSmallLetters = true, bool useCapitalLetters = true, bool useNumbers = true, bool useSymbols = true, bool useAtLeastOneOfEachCategory = true);

}

#endif // RANDOMUTILS_H
