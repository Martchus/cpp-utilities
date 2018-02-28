#include "../conversion/binaryconversion.h"
#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../tests/testutils.h"

using namespace TestUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <functional>
#include <initializer_list>
#include <random>
#include <sstream>

using namespace std;
using namespace ConversionUtilities;

using namespace CPPUNIT_NS;

// compile-time checks for binary conversion
static_assert(toSynchsafeInt(255) == 383, "toSynchsafeInt()");
static_assert(toNormalInt(383) == 255, "toNormalInt()");
static_assert(swapOrder(static_cast<uint16>(0xABCD)) == 0xCDAB, "swapOrder(uint16)");
static_assert(swapOrder(static_cast<uint32>(0xABCDEF12)) == 0x12EFCDAB, "swapOrder(uint32)");
static_assert(swapOrder(static_cast<uint64>(0xABCDEF1234567890)) == 0x9078563412EFCDAB, "swapOrder(uint64)");

/*!
 * \brief The ConversionTests class tests classes and methods of the ConversionUtilities namespace.
 */
class ConversionTests : public TestFixture {
    CPPUNIT_TEST_SUITE(ConversionTests);
    CPPUNIT_TEST(testConversionException);
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

    void testConversionException();
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

void ConversionTests::testConversionException()
{
    CPPUNIT_ASSERT(!strcmp("unable to convert", ConversionException().what()));
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
    CPPUNIT_ASSERT_THROW(convertString("invalid charset", "UTF-8", "foo", 3, 1.0f), ConversionException);
}

/*!
 * \brief Tests miscellaneous string conversions.
 */
void ConversionTests::testStringConversions()
{
    // stringToNumber() / numberToString() with zero and random numbers
    CPPUNIT_ASSERT_EQUAL("0"s, numberToString<unsigned int>(0));
    CPPUNIT_ASSERT_EQUAL("0"s, numberToString<signed int>(0));
    uniform_int_distribution<int64> randomDistSigned(numeric_limits<int64>::min());
    uniform_int_distribution<uint64> randomDistUnsigned(0);
    const string stringMsg("string"), wideStringMsg("wide string"), bufferMsg("buffer");
    for (byte b = 1; b < 100; ++b) {
        auto signedRandom = randomDistSigned(m_randomEngine);
        auto unsignedRandom = randomDistUnsigned(m_randomEngine);
        for (const auto base : initializer_list<byte>{ 2, 8, 10, 16 }) {
            const auto asString = numberToString<uint64, string>(unsignedRandom, static_cast<string::value_type>(base));
            const auto asWideString = numberToString<uint64, wstring>(unsignedRandom, base);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(stringMsg, unsignedRandom, stringToNumber<uint64>(asString, static_cast<string::value_type>(base)));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(wideStringMsg, unsignedRandom, stringToNumber<uint64>(asWideString, base));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(bufferMsg, unsignedRandom, bufferToNumber<uint64>(asString.data(), asString.size(), base));
        }
        for (const auto base : initializer_list<byte>{ 10 }) {
            const auto asString = numberToString<int64, string>(signedRandom, static_cast<string::value_type>(base));
            const auto asWideString = numberToString<int64, wstring>(signedRandom, base);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(stringMsg, signedRandom, stringToNumber<int64>(asString, static_cast<string::value_type>(base)));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(wideStringMsg, signedRandom, stringToNumber<int64>(asWideString, base));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(bufferMsg, signedRandom, bufferToNumber<int64>(asString.data(), asString.size(), base));
        }
    }

    // stringToNumber() with spaces at the beginning, leading zeroes, different types and other corner cases
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<int32>("01"));
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<int32>(L"01"s));
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<int32>(u"01"s));
    CPPUNIT_ASSERT_EQUAL(-23, stringToNumber<int32>(" - 023"s));
    CPPUNIT_ASSERT_EQUAL(-23, bufferToNumber<int32>(" - 023", 6));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<uint32>("01"));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<uint32>(L"01"s));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<uint32>(u"01"s));
    CPPUNIT_ASSERT_EQUAL(23u, stringToNumber<uint32>("  023"s));
    CPPUNIT_ASSERT_EQUAL(23u, bufferToNumber<uint32>("  023", 5));
    CPPUNIT_ASSERT_EQUAL(255u, stringToNumber<uint32>("fF", 16));
    CPPUNIT_ASSERT_THROW(stringToNumber<uint32>("fF", 15), ConversionException);
    CPPUNIT_ASSERT_THROW(stringToNumber<uint32>("(", 15), ConversionException);

    // interpretIntegerAsString()
    CPPUNIT_ASSERT_EQUAL("TEST"s, interpretIntegerAsString<uint32>(0x54455354));

    // splitString() / joinStrings()
    vector<string> splitTestExpected({ "1", "2,3" });
    vector<string> splitTestActual = splitString<vector<string>>("1,2,3"s, ","s, EmptyPartsTreat::Keep, 2);
    CPPUNIT_ASSERT_EQUAL(splitTestExpected, splitTestActual);
    splitTestActual = splitStringSimple<vector<string>>("1,2,3"s, ","s, 2);
    CPPUNIT_ASSERT_EQUAL(splitTestExpected, splitTestActual);
    splitTestExpected = { "1", "2,3", "4,,5" };
    splitTestActual = splitString<vector<string>>("1,2,,3,4,,5"s, ","s, EmptyPartsTreat::Merge, 3);
    CPPUNIT_ASSERT_EQUAL(splitTestExpected, splitTestActual);
    string splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Keep), " "s, false, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("() (a) () (ab) (ABC) (s)"s, splitJoinTest);
    splitJoinTest = joinStrings(splitString<vector<string>>(",a,,ab,ABC,s"s, ","s, EmptyPartsTreat::Keep), " "s, true, "("s, ")"s);
    CPPUNIT_ASSERT_EQUAL("(a) (ab) (ABC) (s)"s, splitJoinTest);
    splitJoinTest = joinStrings(splitStringSimple<vector<string>>(",a,,ab,ABC,s"s, ","s), " "s, true, "("s, ")"s);
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
    CPPUNIT_ASSERT(!startsWith<string>(findReplaceTest, "findAnd"s));
    CPPUNIT_ASSERT(startsWith<string>(findReplaceTest, "findOr"s));

    // containsSubstrings()
    CPPUNIT_ASSERT(containsSubstrings<string>("this string contains foo and bar", { "foo", "bar" }));
    CPPUNIT_ASSERT(!containsSubstrings<string>("this string contains foo and bar", { "bar", "foo" }));

    // truncateString()
    string truncateTest("foo  bar        ");
    truncateString(truncateTest, ' ');
    CPPUNIT_ASSERT_EQUAL("foo"s, truncateTest);

    // encodeBase64() / decodeBase64() with random data
    uniform_int_distribution<byte> randomDistChar;
    byte originalBase64Data[4047];
    for (byte &c : originalBase64Data) {
        c = randomDistChar(m_randomEngine);
    }
    auto encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data));
    auto decodedBase64Data = decodeBase64(encodedBase64Data.data(), static_cast<uint32>(encodedBase64Data.size()));
    CPPUNIT_ASSERT(decodedBase64Data.second == sizeof(originalBase64Data));
    for (unsigned int i = 0; i < sizeof(originalBase64Data); ++i) {
        CPPUNIT_ASSERT(decodedBase64Data.first[i] == originalBase64Data[i]);
    }
    // test padding
    encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data) - 1);
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 1));
    CPPUNIT_ASSERT_NO_THROW(decodeBase64(encodedBase64Data.data(), static_cast<uint32>(encodedBase64Data.size())));
    encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data) - 2);
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 1));
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 2));
    CPPUNIT_ASSERT_NO_THROW(decodeBase64(encodedBase64Data.data(), static_cast<uint32>(encodedBase64Data.size())));
    // test check for invalid size
    CPPUNIT_ASSERT_THROW(decodeBase64(encodedBase64Data.data(), 3), ConversionException);

    // dataSizeToString(), bitrateToString()
    CPPUNIT_ASSERT_EQUAL("512 bytes"s, dataSizeToString(512ul));
    CPPUNIT_ASSERT_EQUAL("2.50 KiB"s, dataSizeToString((2048ul + 512ul)));
    CPPUNIT_ASSERT_EQUAL("2.50 KiB (2560 byte)"s, dataSizeToString((2048ul + 512ul), true));
    CPPUNIT_ASSERT_EQUAL("2.50 MiB"s, dataSizeToString((2048ul + 512ul) * 1024ul));
    CPPUNIT_ASSERT_EQUAL("2.50 GiB"s, dataSizeToString((2048ul + 512ul) * 1024ul * 1024ul));
    CPPUNIT_ASSERT_EQUAL("2.50 TiB"s, dataSizeToString((2048ul + 512ul) * 1024ul * 1024ul * 1024ul));
    CPPUNIT_ASSERT_EQUAL("128 bit/s"s, bitrateToString(0.128, false));
    CPPUNIT_ASSERT_EQUAL("128 kbit/s"s, bitrateToString(128.0, false));
    CPPUNIT_ASSERT_EQUAL("128 Mbit/s"s, bitrateToString(128.0 * 1e3, false));
    CPPUNIT_ASSERT_EQUAL("128 Gbit/s"s, bitrateToString(128.0 * 1e6, false));
    CPPUNIT_ASSERT_EQUAL("16 byte/s"s, bitrateToString(0.128, true));
    CPPUNIT_ASSERT_EQUAL("16 KiB/s"s, bitrateToString(128.0, true));
    CPPUNIT_ASSERT_EQUAL("16 MiB/s"s, bitrateToString(128.0 * 1e3, true));
    CPPUNIT_ASSERT_EQUAL("16 GiB/s"s, bitrateToString(128.0 * 1e6, true));
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
