#include "random.h"

#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cassert>

using namespace std;

/*!
 * \namespace MiscUtilities
 * \brief Contains miscellaneous utility functions.
 */

namespace Utilities
{

//! @cond
const char letters[28] = "qwertzuiopasdfghjklyxcvbnm";
const char capitalLetters[28] = "QWERTZUIOPASDFGHJKLYXCVBNM";
const char numbers[11] = "1234567890";
const char symbols[24] = "!\"$%&/()=?'#*+~-_><.:,;";
//! @endcond

/*!
 * Generates a random character sequence using the given \a randomizer.
 */
void generateRandomCharacterSequence(char *result, int length, std::function<int ()> randomizer, int highestRandomNumber, bool useSmallLetters, bool useCapitalLetters, bool useNumbers, bool useSymbols, bool useAtLeastOneOfEachCategory)
{
    if(length <= 0) {
        return;
    }
    int categoryCount = 0;
    int neededCharacters;
    bool needSmallLetter = false;
    bool needCapitalLetter = false;
    bool needNumber = false;
    bool needSymbol = false;
    if(useSmallLetters) {
        needSmallLetter = useAtLeastOneOfEachCategory;
        ++categoryCount;
    }
    if(useCapitalLetters) {
        needCapitalLetter = useAtLeastOneOfEachCategory;
        ++categoryCount;
    }
    if(useNumbers) {
        needNumber = useAtLeastOneOfEachCategory;
        ++categoryCount;
    }
    if(useSymbols) {
        needSymbol = useAtLeastOneOfEachCategory;
        ++categoryCount;
    }
    neededCharacters = useAtLeastOneOfEachCategory ? categoryCount : 0;
    if(categoryCount <= 0) {
        result[0] = '\0';
        return;
    }
    for(int i = 0; i < length; ++i) {
        int category = -1;
        if((neededCharacters > 0 && (randomizer() < (highestRandomNumber / 2.0))) || (neededCharacters + i >= length)) {
            if(needSmallLetter)
                category = 0;
            if(needCapitalLetter && ((category == -1) || (randomizer() < (highestRandomNumber / 2.0))))
                category = 1;
            if(needNumber && ((category == -1) || (randomizer() < (highestRandomNumber / 4.0))))
                category = 2;
            if(needSymbol && ((category == -1) || (randomizer() < (highestRandomNumber / 8.0))))
                category =  3;
        } else {
            if(useSmallLetters)
                category = 0;
            if(useCapitalLetters && ((category == -1) || (randomizer() < (highestRandomNumber / 2.0))))
                category = 1;
            if(useNumbers && ((category == -1) || (randomizer() < (highestRandomNumber / 4.0))))
                category = 2;
            if(useSymbols && ((category == -1) || (randomizer() < (highestRandomNumber / 8.0))))
                category =  3;
        }
        switch(category) {
        case 0:
            result[i] = letters[rand() % 26];
            if(needSmallLetter) {
                needSmallLetter = false;
                --neededCharacters;
            }
            break;
        case 1:
            result[i] = capitalLetters[rand() % 26];
            if(needCapitalLetter) {
                needCapitalLetter = false;
                --neededCharacters;
            }
            break;
        case 2:
            result[i] = numbers[rand() % 9];
            if(needNumber) {
                needNumber = false;
                --neededCharacters;
            }
            break;
        case 3:
            result[i] = symbols[rand() % 22];
            if(needSymbol) {
                needSymbol = false;
                --neededCharacters;
            }
            break;
        }
    }
}

/*!
 * Generates a random character sequence using std::rand().
 */
void generateRandomCharacterSequence(char *result, int length, bool useSmallLetters, bool useCapitalLetters, bool useNumbers, bool useSymbols, bool useAtLeastOneOfEachCategory)
{
    generateRandomCharacterSequence(result, length, rand, RAND_MAX, useSmallLetters, useCapitalLetters, useNumbers, useSymbols, useAtLeastOneOfEachCategory);
}

}

