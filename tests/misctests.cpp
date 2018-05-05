#include "../misc/levenshtein.h"
#include "../misc/multiarray.h"
#include "../tests/testutils.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace MiscUtilities;
using namespace TestUtilities::Literals;

using namespace CPPUNIT_NS;

/*!
 * \brief The MiscTests class tests functions and classes from the misc directory.
 */
class MiscTests : public TestFixture {
    CPPUNIT_TEST_SUITE(MiscTests);
    CPPUNIT_TEST(testMultiArray);
    CPPUNIT_TEST(testLevenshtein);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }
    void tearDown()
    {
    }

    void testMultiArray();
    void testLevenshtein();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MiscTests);

void MiscTests::testMultiArray()
{
    static_assert(decltype(makeMultiArray<char>(3))::dimensionCount() == 1, "dimension count 1D");
    static_assert(decltype(makeMultiArray<char>(3, 2))::dimensionCount() == 2, "dimension count 2D");
    static_assert(decltype(makeMultiArray<char>(3, 2, 3))::dimensionCount() == 3, "dimension count 3D");

    auto array1d(makeMultiArray<char>(3));
    CPPUNIT_ASSERT_EQUAL(3_st, array1d.dimensionSize<0>());
    CPPUNIT_ASSERT_EQUAL(3_st, array1d.totalSize());
    array1d.at(0) = 'a';
    array1d.at(1) = 'b';
    array1d.at(2) = 'c';
    CPPUNIT_ASSERT_EQUAL("abc"s, string(array1d.data(), 3));

    auto array2d(makeMultiArray<char>(3, 2));
    CPPUNIT_ASSERT_EQUAL(3_st, array2d.dimensionSize<0>());
    CPPUNIT_ASSERT_EQUAL(2_st, array2d.dimensionSize<1>());
    CPPUNIT_ASSERT_EQUAL(6_st, array2d.totalSize());
    const char *const data(array2d.data());
    array2d.at(0, 0) = 'a';
    array2d.at(0, 1) = 'b';
    array2d.at(1, 0) = 'c';
    array2d.at(1, 1) = 'd';
    array2d.at(2, 0) = 'e';
    array2d.at(2, 1) = 'f';
    CPPUNIT_ASSERT_EQUAL("abcdef"s, string(data, 6));

    auto array3d(makeMultiArray<char>(3, 2, 3));
    CPPUNIT_ASSERT_EQUAL(3_st, array3d.dimensionSize<0>());
    CPPUNIT_ASSERT_EQUAL(2_st, array3d.dimensionSize<1>());
    CPPUNIT_ASSERT_EQUAL(3_st, array3d.dimensionSize<2>());
    CPPUNIT_ASSERT_EQUAL(18_st, array3d.totalSize());
    array3d.at(0, 0, 0) = 'a';
    array3d.at(0, 0, 1) = 'b';
    array3d.at(0, 0, 2) = 'c';
    array3d.at(0, 1, 0) = 'd';
    array3d.at(0, 1, 1) = 'e';
    array3d.at(0, 1, 2) = 'f';
    array3d.at(1, 0, 0) = 'g';
    array3d.at(1, 0, 1) = 'h';
    array3d.at(1, 0, 2) = 'i';
    array3d.at(1, 1, 0) = 'j';
    array3d.at(1, 1, 1) = 'k';
    array3d.at(1, 1, 2) = 'l';
    array3d.at(2, 0, 0) = 'm';
    array3d.at(2, 0, 1) = 'n';
    array3d.at(2, 0, 2) = 'o';
    array3d.at(2, 1, 0) = 'p';
    array3d.at(2, 1, 1) = 'q';
    array3d.at(2, 1, 2) = 'r';
    CPPUNIT_ASSERT_EQUAL("abcdefghijklmnopqr"s, string(array3d.data(), 18));

    auto stackMultiArray(makeFixedSizeMultiArray<char, 9>(3, 3));
    CPPUNIT_ASSERT_EQUAL(3_st, stackMultiArray.dimensionSize<0>());
    CPPUNIT_ASSERT_EQUAL(3_st, stackMultiArray.dimensionSize<1>());
    CPPUNIT_ASSERT_EQUAL(9_st, stackMultiArray.totalSize());
    stackMultiArray.at(0, 0) = 'a';
    stackMultiArray.at(0, 1) = 'b';
    stackMultiArray.at(0, 2) = 'c';
    stackMultiArray.at(1, 0) = 'd';
    stackMultiArray.at(1, 1) = 'e';
    stackMultiArray.at(1, 2) = 'f';
    stackMultiArray.at(2, 0) = 'g';
    stackMultiArray.at(2, 1) = 'h';
    stackMultiArray.at(2, 2) = 'i';
    CPPUNIT_ASSERT_EQUAL("abcdefghi"s, string(stackMultiArray.data(), 9));
}

void MiscTests::testLevenshtein()
{
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("ab", "abc"));
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("abc", "ab"));
    CPPUNIT_ASSERT_EQUAL(2_st, computeDamerauLevenshteinDistance("xzaby", "xbay"));
    CPPUNIT_ASSERT_EQUAL(0_st, computeDamerauLevenshteinDistance("", ""));
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("ab", "ba"));
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("xaby", "xbay"));
    CPPUNIT_ASSERT_EQUAL(0_st, computeDamerauLevenshteinDistance("abc", "abc"));
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("ab", "abc"));
    CPPUNIT_ASSERT_EQUAL(2_st, computeDamerauLevenshteinDistance("ca", "abc"));
    CPPUNIT_ASSERT_EQUAL(4_st, computeDamerauLevenshteinDistance("", "abcd"));
    CPPUNIT_ASSERT_EQUAL(4_st, computeDamerauLevenshteinDistance("abcd", ""));
    CPPUNIT_ASSERT_EQUAL(3_st, computeDamerauLevenshteinDistance("abcd", "d"));
    CPPUNIT_ASSERT_EQUAL(2_st, computeDamerauLevenshteinDistance("abcd", "bc"));
    CPPUNIT_ASSERT_EQUAL(3_st, computeDamerauLevenshteinDistance("abcd", "a"));
    CPPUNIT_ASSERT_EQUAL(2_st, computeDamerauLevenshteinDistance("adb", "abc"));
    CPPUNIT_ASSERT_EQUAL(2_st, computeDamerauLevenshteinDistance("xxaxx", "xxäxx"));
    CPPUNIT_ASSERT_EQUAL(1_st, computeDamerauLevenshteinDistance("xxöxx", "xxäxx"));
    CPPUNIT_ASSERT_EQUAL(11_st, computeDamerauLevenshteinDistance("this is a long text", "this is too long for stack"));
}
