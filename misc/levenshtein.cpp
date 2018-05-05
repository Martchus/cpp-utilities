#include "./levenshtein.h"
#include "./multiarray.h"

#include "../math/math.h"

#include <iostream>
#include <limits>
#include <memory>

using namespace std;

/*!
 * \namespace MiscUtilities
 * \brief The MiscUtilities namespace contains various utilities such as computing Damerau–Levenshtein distance
 *        and *N*-dimensional arrays.
 */
namespace MiscUtilities {

/// \cond

/// \brief The DistanceArray is a 2D array which is allocated either on the stack or the heap.
using DistanceArray = MultiArray<size_t, NoneOwningMultiArray, size_t, size_t>;

/// \brief Initializes to distance array.
///
/// For size1=5 and size2=4 it would look like this:
/// ```
///  9 9 9 9 9 9
///  9 0 1 2 3 4
///  9 1 0 0 0 0
///  9 2 0 0 0 0
///  9 3 0 0 0 0
///  9 4 0 0 0 0
///  9 5 0 0 0 0
/// ```
void initDistanceArray(DistanceArray &distanceArray, const size_t size1, const size_t size2)
{
    const auto maxDistance(size1 + size2);
    distanceArray.at(0, 0) = maxDistance;
    for (size_t i = 0; i <= size1; ++i) {
        distanceArray.at(i + 1, 1) = i;
        distanceArray.at(i + 1, 0) = maxDistance;
    }
    for (size_t i = 0; i <= size2; ++i) {
        distanceArray.at(1, i + 1) = i;
        distanceArray.at(0, i + 1) = maxDistance;
    }
}

/// \brief Performs the actual Damerau–Levenshtein algorithm.
/// \sa computeDamerauLevenshteinDistance() for more details on the algorithm.
size_t performDamerauLevenshteinAlgorithm(
    DistanceArray &distanceArray, const char *const str1, const size_t size1, const char *const str2, const size_t size2)
{
    size_t dist1[std::numeric_limits<unsigned char>::max() + 1] = { 0 };
    for (size_t index1 = 1; index1 <= size1; ++index1) {
        size_t dist2 = 0;
        for (size_t index2 = 1; index2 <= size2; ++index2) {
            const size_t substitution((str1[index1 - 1] == str2[index2 - 1]) ? 0 : 1);
            const size_t transposition1(dist1[static_cast<unsigned char>(str2[index2 - 1])]);
            const size_t transposition2(dist2);
            if (!substitution) {
                dist2 = index2;
            }
            // clang-format off
            distanceArray.at(index1 + 1, index2 + 1) = MathUtilities::min(
                distanceArray.at(index1, index2) + substitution,                                                                     // substitution
                distanceArray.at(index1 + 1, index2) + 1,                                                                            // insertion
                distanceArray.at(index1, index2 + 1) + 1,                                                                            // deletion
                distanceArray.at(transposition1, transposition2) + (index1 - transposition1 - 1) + 1 + (index2 - transposition2 - 1) // transposition
            );
            // clang-format on
        }
        dist1[static_cast<int>(str1[index1 - 1])] = index1;
    }
    return distanceArray.at(size1 + 1, size2 + 1);
}

/// \brief Allocates the distance array on the heap and performs the Damerau–Levenshtein algorithm.
template <typename DistanceArray>
size_t performDamerauLevenshteinAlgorithmAllocatingOnHeap(
    DistanceArray &distanceArray, const char *const str1, const size_t size1, const char *const str2, const size_t size2)
{
    std::vector<size_t> buffer(distanceArray.totalSize());
    distanceArray.buffer() = buffer.data();
    initDistanceArray(distanceArray, size1, size2);
    return performDamerauLevenshteinAlgorithm(distanceArray, str1, size1, str2, size2);
}

/// \brief Allocates the distance array on the stack and performs the Damerau–Levenshtein algorithm.
/// \remarks The totalSize() of \a distanceArray mustn't exceed 128 byte.
template <typename DistanceArray>
size_t performDamerauLevenshteinAlgorithmAllocatingOnStack(
    DistanceArray &distanceArray, const char *const str1, const size_t size1, const char *const str2, const size_t size2)
{
    size_t buffer[128] = { 0 };
    distanceArray.buffer() = buffer;
    initDistanceArray(distanceArray, size1, size2);
    return performDamerauLevenshteinAlgorithm(distanceArray, str1, size1, str2, size2);
}

/// \endcond

/*!
 * \brief Computes Damerau–Levenshtein distance with adjacent transpositions.
 *
 * \returns Returns the number of editing steps required to turn \a str1 into \a str2.
 * The following operations are considered as editing steps:
 * - substitution: replace one character with another character
 * - insertion: insert one character at any position
 * - deletion: delete one character at any position
 * - transposition: swap any pair of adjacent characters
 *
 * \remarks
 * - Computing Optimal string alignment distance is a *different* thing.
 * - The algorithm operates on byte-level. So characters requiring more than one byte in
 *   the used character encoding (eg. UTF-8 encoded German umlauts) are counted as multiple
 *   characters (eg. substitution of those umlauts with non-umlauts requires 2 editing steps).
 * - The memory consumption of this algorithm is considerably. The required memory increases
 *   with the product of \a size1 and \a size2. Pass only short words to this function!
 */
std::size_t computeDamerauLevenshteinDistance(const char *const str1, const size_t size1, const char *const str2, const size_t size2)
{
    // allocate distance array
    auto distanceArray(makeNoneOwningMultiArray<std::size_t>(size1 + 2, size2 + 2));
    if (distanceArray.totalSize() <= 128) {
        return performDamerauLevenshteinAlgorithmAllocatingOnStack(distanceArray, str1, size1, str2, size2);
    } else {
        return performDamerauLevenshteinAlgorithmAllocatingOnHeap(distanceArray, str1, size1, str2, size2);
    }
}

} // namespace MiscUtilities
