#include "../conversion/binaryconversion.h"
#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../tests/testutils.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <functional>
#include <initializer_list>
#include <random>
#include <sstream>

using namespace std;
using namespace ConversionUtilities;
using namespace TestUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The ConversionTests class tests classes and methods of the ConversionUtilities namespace.
 */
class ConversionTests : public TestFixture {
    CPPUNIT_TEST_SUITE(ConversionTests);
    CPPUNIT_TEST(testEndianness);
    CPPUNIT_TEST(testBinaryConversions);
    CPPUNIT_TEST(testSwapOrderFunctions);
    CPPUNIT_TEST(testStringEncodingConversions);
    CPPUNIT_TEST(testStringConversions);
    CPPUNIT_TEST(testStringBuilder);
    CPPUNIT_TEST_SUITE_END();

public:
    ConversionTests();

    void setUp()
    {
    }
    void tearDown()
    {
    }

    void testEndianness();
    void testBinaryConversions();
    void testSwapOrderFunctions();
    void testStringEncodingConversions();
    void testStringConversions();
    void testStringBuilder();

private:
    template <typename intType>
    void testConversion(const char *message, function<void(intType, char *)> vice, function<intType(const char *)> verca, intType min, intType max);

    char m_buff[8];
    random_device m_randomDevice;
    mt19937 m_randomEngine;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConversionTests);

ConversionTests::ConversionTests()
    : m_randomDevice()
    , m_randomEngine(m_randomDevice())
{
}

/*!
 * \brief Tests whether macros for endianness are correct.
 */
void ConversionTests::testEndianness()
{
    union {
        uint32_t integer;
        char characters[4];
    } test = { 0x01020304 };
#if defined(CONVERSION_UTILITIES_BYTE_ORDER_BIG_ENDIAN)
    // test whether macro definitions are consistent
    CPPUNIT_ASSERT(CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN == true);
    CPPUNIT_ASSERT(CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN == false);
    // test whether byte order assumption is correct
    CPPUNIT_ASSERT_MESSAGE("Byte order assumption (big-endian) is wrong", test.characters[0] == 0x01);
#elif defined(CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN)
    // test whether macro definitions are consistent
    CPPUNIT_ASSERT(CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN == false);
    CPPUNIT_ASSERT(CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN == true);
    // test whether byte order assumption is correct
    CPPUNIT_ASSERT_MESSAGE("Byte order assumption (little-endian) is wrong", test.characters[0] == 0x04);
#else
    CPPUNIT_FAIL("There is not valid byte order assumption");
#endif
}

template <typename intType>
void ConversionTests::testConversion(
    const char *message, function<void(intType, char *)> vice, function<intType(const char *)> versa, intType min, intType max)
{
    const intType random = uniform_int_distribution<intType>(min, max)(m_randomEngine);
    stringstream msg;
    msg << message << '(' << hex << '0' << 'x' << random << ')';
    vice(random, m_buff);
    CPPUNIT_ASSERT_MESSAGE(msg.str(), versa(m_buff) == random);
}

#define TEST_TYPE(endianness, function) decltype(endianness::function(m_buff))

#define TEST_CONVERSION(function, endianness)                                                                                                        \
    testConversion<TEST_TYPE(endianness, function)>("testing " #function,                                                                            \
        static_cast<void (*)(TEST_TYPE(endianness, function), char *)>(&endianness::getBytes), endianness::function,                                 \
        numeric_limits<TEST_TYPE(endianness, function)>::min(), numeric_limits<TEST_TYPE(endianness, function)>::max())

#define TEST_BE_CONVERSION(function) TEST_CONVERSION(function, BE)

#define TEST_LE_CONVERSION(function) TEST_CONVERSION(function, LE)

#define TEST_CUSTOM_CONVERSION(vice, versa, endianness, min, max)                                                                                    \
    testConversion<TEST_TYPE(endianness, versa)>(                                                                                                    \
        "testing " #versa, static_cast<void (*)(TEST_TYPE(endianness, versa), char *)>(&endianness::vice), endianness::versa, min, max)

/*!
 * \brief Tests most important binary conversions.
 *
 * Tests toUInt16(), ... toUInt64(), toInt16(), ... toInt64() and
 * the inverse getBytes() functions with random numbers.
 */
void ConversionTests::testBinaryConversions()
{
    // test to...() / getBytes() with random numbers
    for (byte b = 1; b < 100; ++b) {
        TEST_BE_CONVERSION(toUInt16);
        TEST_BE_CONVERSION(toUInt32);
        TEST_BE_CONVERSION(toUInt64);
        TEST_LE_CONVERSION(toUInt16);
        TEST_LE_CONVERSION(toUInt32);
        TEST_LE_CONVERSION(toUInt64);
        TEST_BE_CONVERSION(toInt16);
        TEST_BE_CONVERSION(toInt32);
        TEST_BE_CONVERSION(toInt64);
        TEST_LE_CONVERSION(toInt16);
        TEST_LE_CONVERSION(toInt32);
        TEST_LE_CONVERSION(toInt64);
        TEST_CUSTOM_CONVERSION(getBytes24, toUInt24, BE, 0, 0xFFFFFF);
        TEST_CUSTOM_CONVERSION(getBytes24, toUInt24, LE, 0, 0xFFFFFF);
    }
}

/*!
 * \brief Tests swap order functions.
 */
void ConversionTests::testSwapOrderFunctions()
{
    CPPUNIT_ASSERT(swapOrder(static_cast<uint16>(0x7825)) == 0x2578);
    CPPUNIT_ASSERT(swapOrder(static_cast<uint32>(0x12345678)) == 0x78563412);
    CPPUNIT_ASSERT(swapOrder(static_cast<uint64>(0x1122334455667788)) == 0x8877665544332211);
}

/*!
 * \brief Internally used for string encoding tests to check results.
 */
void assertEqual(const char *message, const byte *expectedValues, size_t expectedSize, const StringData &actualValues)
{
    // check whether number of elements matches
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expectedSize, actualValues.second);
    // check whether contents match
    auto *end = expectedValues + expectedSize;
    auto *i = reinterpret_cast<byte *>(actualValues.first.get());
    for (; expectedValues != end; ++expectedValues, ++i) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(message, asHexNumber(*expectedValues), asHexNumber(*i));
    }
}

#if CONVERSION_UTILITIES_IS_BYTE_ORDER_LITTLE_ENDIAN == true
#define LE_STR_FOR_ENDIANNESS(name) name##LE##String
#define BE_STR_FOR_ENDIANNESS(name) name##BE##String
#elif CONVERSION_UTILITIES_IS_BYTE_ORDER_BIG_ENDIAN == true
#define LE_STR_FOR_ENDIANNESS(name) name##BE##String
#define BE_STR_FOR_ENDIANNESS(name) name##LE##String
#endif

/*!
 * \def LE_STR_FOR_ENDIANNESS
 * \brief Selects right string for little-endian checks.
 */

/*!
 * \def BE_STR_FOR_ENDIANNESS
 * \brief Selects right string for big-endian checks.
 */

/*!
 * \brief Tests string encoding conversions.
 */
void ConversionTests::testStringEncodingConversions()
{
    // define test string "ABCD" for the different encodings
    const byte simpleString[] = { 'A', 'B', 'C', 'D' };
    const uint16 simpleUtf16LEString[] = { 0x0041, 0x0042, 0x0043, 0x0044 };
    const uint16 simpleUtf16BEString[] = { 0x4100, 0x4200, 0x4300, 0x4400 };
    // define test string "ABÖCD" for the different encodings
    const byte latin1String[] = { 'A', 'B', 0xD6, 'C', 'D' };
    const byte utf8String[] = { 'A', 'B', 0xC3, 0x96, 'C', 'D' };
    const uint16 utf16LEString[] = { 0x0041, 0x0042, 0x00D6, 0x0043, 0x0044 };
    const uint16 utf16BEString[] = { 0x4100, 0x4200, 0xD600, 0x4300, 0x4400 };
    // test conversion to UTF-8
    assertEqual("Latin-1 to UTF-8 (simple)", simpleString, 4, convertLatin1ToUtf8(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("Latin-1 to UTF-8", utf8String, 6, convertLatin1ToUtf8(reinterpret_cast<const char *>(latin1String), 5));
    assertEqual(
        "UTF-16LE to UTF-8 (simple)", simpleString, 4, convertUtf16LEToUtf8(reinterpret_cast<const char *>(LE_STR_FOR_ENDIANNESS(simpleUtf16)), 8));
    assertEqual("UTF-16LE to UTF-8", utf8String, 6, convertUtf16LEToUtf8(reinterpret_cast<const char *>(LE_STR_FOR_ENDIANNESS(utf16)), 10));
    assertEqual(
        "UTF-16BE to UTF-8 (simple)", simpleString, 4, convertUtf16BEToUtf8(reinterpret_cast<const char *>(BE_STR_FOR_ENDIANNESS(simpleUtf16)), 8));
    assertEqual("UTF-16BE to UTF-8", utf8String, 6, convertUtf16BEToUtf8(reinterpret_cast<const char *>(BE_STR_FOR_ENDIANNESS(utf16)), 10));
    // test conversion from UTF-8
    assertEqual("UTF-8 to Latin-1 (simple)", simpleString, 4, convertUtf8ToLatin1(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("UTF-8 to Latin-1", latin1String, 5, convertUtf8ToLatin1(reinterpret_cast<const char *>(utf8String), 6));
    assertEqual("UTF-8 to UFT-16LE (simple)", reinterpret_cast<const byte *>(LE_STR_FOR_ENDIANNESS(simpleUtf16)), 8,
        convertUtf8ToUtf16LE(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("UTF-8 to UFT-16LE", reinterpret_cast<const byte *>(LE_STR_FOR_ENDIANNESS(utf16)), 10,
        convertUtf8ToUtf16LE(reinterpret_cast<const char *>(utf8String), 6));
    assertEqual("UTF-8 to UFT-16BE (simple)", reinterpret_cast<const byte *>(BE_STR_FOR_ENDIANNESS(simpleUtf16)), 8,
        convertUtf8ToUtf16BE(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("UTF-8 to UFT-16BE", reinterpret_cast<const byte *>(BE_STR_FOR_ENDIANNESS(utf16)), 10,
        convertUtf8ToUtf16BE(reinterpret_cast<const char *>(utf8String), 6));
}

/*!
 * \brief Tests miscellaneous string conversions.
 */
void ConversionTests::testStringConversions()
{
    // stringToNumber() / numberToString() with zero and random numbers
    CPPUNIT_ASSERT_EQUAL(string("0"), numberToString<unsigned int>(0));
    CPPUNIT_ASSERT_EQUAL(string("0"), numberToString<signed int>(0));
    uniform_int_distribution<int64> randomDistSigned(numeric_limits<int64>::min());
    uniform_int_distribution<uint64> randomDistUnsigned(0);
    for (byte b = 1; b < 100; ++b) {
        auto signedRandom = randomDistSigned(m_randomEngine);
        auto unsignedRandom = randomDistUnsigned(m_randomEngine);
        for (const auto base : initializer_list<byte>{ 2, 8, 10, 16 }) {
            auto resultString = stringToNumber<uint64, string>(numberToString<uint64, string>(unsignedRandom, base), base);
            auto resultWideString = stringToNumber<uint64, wstring>(numberToString<uint64, wstring>(unsignedRandom, base), base);
            CPPUNIT_ASSERT_EQUAL(unsignedRandom, resultString);
            CPPUNIT_ASSERT_EQUAL(unsignedRandom, resultWideString);
        }
        for (const auto base : initializer_list<byte>{ 10 }) {
            auto resultString = stringToNumber<int64, string>(numberToString<int64, string>(signedRandom, base), base);
            auto resultWideString = stringToNumber<int64, wstring>(numberToString<int64, wstring>(signedRandom, base), base);
            CPPUNIT_ASSERT_EQUAL(signedRandom, resultString);
            CPPUNIT_ASSERT_EQUAL(signedRandom, resultWideString);
        }
    }

    // stringToNumber() with leading zeroes and different types
    int32 res = stringToNumber<int32, string>("01");
    CPPUNIT_ASSERT_EQUAL(1, res);
    res = stringToNumber<int32, wstring>(L"01");
    CPPUNIT_ASSERT_EQUAL(1, res);
    res = stringToNumber<int32, u16string>(u"01");
    CPPUNIT_ASSERT_EQUAL(1, res);

    // interpretIntegerAsString()
    CPPUNIT_ASSERT(interpretIntegerAsString<uint32>(0x54455354) == "TEST");

    // splitString() / joinStrings()
    string splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Keep), " "s, false, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("() (a) () (ab) (ABC) (s)"s, splitJoinTest);
    splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Keep), " "s, true, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("(a) (ab) (ABC) (s)"s, splitJoinTest);
    splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Omit), " "s, false, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("(a) (ab) (ABC) (s)"s, splitJoinTest);
    splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Merge), " "s, false, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("(a,ab) (ABC) (s)"s, splitJoinTest);

    // findAndReplace()
    string findReplaceTest("findAndReplace()");
    findAndReplace<string>(findReplaceTest, "And", "Or");
    CPPUNIT_ASSERT_EQUAL("findOrReplace()"s, findReplaceTest);

    // startsWith()
    CPPUNIT_ASSERT(!startsWith<string>(findReplaceTest, "findAnd"));
    CPPUNIT_ASSERT(startsWith<string>(findReplaceTest, "findOr"));

    // containsSubstrings()
    CPPUNIT_ASSERT(containsSubstrings<string>("this string contains foo and bar", { "foo", "bar" }));
    CPPUNIT_ASSERT(!containsSubstrings<string>("this string contains foo and bar", { "bar", "foo" }));

    // encodeBase64() / decodeBase64() with random data
    uniform_int_distribution<byte> randomDistChar;
    byte originalBase64Data[4047];
    for (byte &c : originalBase64Data) {
        c = randomDistChar(m_randomEngine);
    }
    const auto encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data));
    auto decodedBase64Data = decodeBase64(encodedBase64Data.data(), encodedBase64Data.size());
    CPPUNIT_ASSERT(decodedBase64Data.second == sizeof(originalBase64Data));
    for (unsigned int i = 0; i < sizeof(originalBase64Data); ++i) {
        CPPUNIT_ASSERT(decodedBase64Data.first[i] == originalBase64Data[i]);
    }
}

string functionTakingString(const string &str)
{
    return str;
}

void ConversionTests::testStringBuilder()
{
    // conversion of string-tuple to string (the actual string builder)
    const tuple<const char *, string, int, const char *> tuple("string1", "string2", 1234, "string3");
    CPPUNIT_ASSERT_EQUAL(string("string1string21234string3"), tupleToString(tuple));
    CPPUNIT_ASSERT_EQUAL(string("foobarfoo2bar2"), tupleToString(string("foo") % "bar" % string("foo2") % "bar2"));
    CPPUNIT_ASSERT_EQUAL(string("v2.3.0"), argsToString("v2.", 3, '.', 0));

    // construction of string-tuple and final conversion to string works
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "result can be passed to any function taking a std::string"s, "123456789"s, functionTakingString("12" % string("34") % '5' % 67 + "89"));
    constexpr double velocityExample = 27.0;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("real-word example"s, "velocity: 27 km/h (7.5 m/s)"s,
        functionTakingString("velocity: " % numberToString(velocityExample) % " km/h (" % numberToString(velocityExample / 3.6) + " m/s)"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "regular + operator still works (no problems with ambiguity)"s, "regular + still works"s, "regular"s + " + still works");
}
