#include "../conversion/binaryconversion.h"
#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../tests/testutils.h"

using namespace CppUtilities;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <functional>
#include <initializer_list>
#include <random>
#include <sstream>

#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
#include <filesystem>
#endif

using namespace std;

using namespace CPPUNIT_NS;

// compile-time checks for binary conversion
static_assert(toSynchsafeInt(255) == 383, "toSynchsafeInt()");
static_assert(toNormalInt(383) == 255, "toNormalInt()");
static_assert(swapOrder(static_cast<std::uint16_t>(0xABCD)) == 0xCDAB, "swapOrder(uint16)");
static_assert(swapOrder(static_cast<std::uint32_t>(0xABCDEF12)) == 0x12EFCDAB, "swapOrder(uint32)");
static_assert(swapOrder(static_cast<std::uint64_t>(0xABCDEF1234567890)) == 0x9078563412EFCDAB, "swapOrder(uint64)");

/*!
 * \brief The ConversionTests class tests classes and functions provided by the files inside the conversion directory.
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
    for (auto b = 1; b < 100; ++b) {
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
    CPPUNIT_ASSERT(swapOrder(static_cast<std::uint16_t>(0x7825)) == 0x2578);
    CPPUNIT_ASSERT(swapOrder(static_cast<std::uint32_t>(0x12345678)) == 0x78563412);
    CPPUNIT_ASSERT(swapOrder(static_cast<std::uint64_t>(0x1122334455667788)) == 0x8877665544332211);
}

/*!
 * \brief Internally used for string encoding tests to check results.
 */
void assertEqual(const char *message, const std::uint8_t *expectedValues, size_t expectedSize, const StringData &actualValues)
{
    // check whether number of elements matches
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expectedSize, actualValues.second);
    // check whether contents match
    auto *end = expectedValues + expectedSize;
    auto *i = reinterpret_cast<std::uint8_t *>(actualValues.first.get());
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
    const std::uint8_t simpleString[] = { 'A', 'B', 'C', 'D' };
    const std::uint16_t simpleUtf16LEString[] = { 0x0041, 0x0042, 0x0043, 0x0044 };
    const std::uint16_t simpleUtf16BEString[] = { 0x4100, 0x4200, 0x4300, 0x4400 };
    // define test string "ABÖCD" for the different encodings
    const std::uint8_t latin1String[] = { 'A', 'B', 0xD6, 'C', 'D' };
    const std::uint8_t utf8String[] = { 'A', 'B', 0xC3, 0x96, 'C', 'D' };
    const std::uint16_t utf16LEString[] = { 0x0041, 0x0042, 0x00D6, 0x0043, 0x0044 };
    const std::uint16_t utf16BEString[] = { 0x4100, 0x4200, 0xD600, 0x4300, 0x4400 };
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
    assertEqual("UTF-8 to UFT-16LE (simple)", reinterpret_cast<const std::uint8_t *>(LE_STR_FOR_ENDIANNESS(simpleUtf16)), 8,
        convertUtf8ToUtf16LE(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("UTF-8 to UFT-16LE", reinterpret_cast<const std::uint8_t *>(LE_STR_FOR_ENDIANNESS(utf16)), 10,
        convertUtf8ToUtf16LE(reinterpret_cast<const char *>(utf8String), 6));
    assertEqual("UTF-8 to UFT-16BE (simple)", reinterpret_cast<const std::uint8_t *>(BE_STR_FOR_ENDIANNESS(simpleUtf16)), 8,
        convertUtf8ToUtf16BE(reinterpret_cast<const char *>(simpleString), 4));
    assertEqual("UTF-8 to UFT-16BE", reinterpret_cast<const std::uint8_t *>(BE_STR_FOR_ENDIANNESS(utf16)), 10,
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
    uniform_int_distribution<std::int64_t> randomDistSigned(numeric_limits<std::int64_t>::min());
    uniform_int_distribution<std::uint64_t> randomDistUnsigned(0);
    const string stringMsg("string"), wideStringMsg("wide string"), bufferMsg("buffer");
    for (std::uint8_t b = 1; b < 100; ++b) {
        auto signedRandom = randomDistSigned(m_randomEngine);
        auto unsignedRandom = randomDistUnsigned(m_randomEngine);
        for (const auto base : initializer_list<std::uint8_t>{ 2, 8, 10, 16 }) {
            const auto asString = numberToString<std::uint64_t, string>(unsignedRandom, static_cast<string::value_type>(base));
            const auto asWideString = numberToString<std::uint64_t, wstring>(unsignedRandom, base);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(stringMsg, unsignedRandom, stringToNumber<std::uint64_t>(asString, static_cast<string::value_type>(base)));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(wideStringMsg, unsignedRandom, stringToNumber<std::uint64_t>(asWideString, base));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(bufferMsg, unsignedRandom, bufferToNumber<std::uint64_t>(asString.data(), asString.size(), base));
        }
        for (const auto base : initializer_list<std::uint8_t>{ 10 }) {
            const auto asString = numberToString<std::int64_t, string>(signedRandom, static_cast<string::value_type>(base));
            const auto asWideString = numberToString<std::int64_t, wstring>(signedRandom, base);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(stringMsg, signedRandom, stringToNumber<std::int64_t>(asString, static_cast<string::value_type>(base)));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(wideStringMsg, signedRandom, stringToNumber<std::int64_t>(asWideString, base));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(bufferMsg, signedRandom, bufferToNumber<std::int64_t>(asString.data(), asString.size(), base));
        }
    }

    // stringToNumber() with spaces at the beginning, leading zeroes, different types and other corner cases
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<std::int32_t>("01"));
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<std::int32_t>(L"01"s));
    CPPUNIT_ASSERT_EQUAL(1, stringToNumber<std::int32_t>(u"01"s));
    CPPUNIT_ASSERT_EQUAL(-23, stringToNumber<std::int32_t>(" - 023"s));
    CPPUNIT_ASSERT_EQUAL(-23, bufferToNumber<std::int32_t>(" - 023", 6));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<std::uint32_t>("01"));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<std::uint32_t>(L"01"s));
    CPPUNIT_ASSERT_EQUAL(1u, stringToNumber<std::uint32_t>(u"01"s));
    CPPUNIT_ASSERT_EQUAL(23u, stringToNumber<std::uint32_t>("  023"s));
    CPPUNIT_ASSERT_EQUAL(23u, bufferToNumber<std::uint32_t>("  023", 5));
    CPPUNIT_ASSERT_EQUAL(255u, stringToNumber<std::uint32_t>("fF", 16));
    CPPUNIT_ASSERT_THROW(stringToNumber<std::uint32_t>("fF", 15), ConversionException);
    CPPUNIT_ASSERT_THROW(stringToNumber<std::uint32_t>("(", 15), ConversionException);

    // interpretIntegerAsString()
    CPPUNIT_ASSERT_EQUAL("TEST"s, interpretIntegerAsString<std::uint32_t>(0x54455354));

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
    CPPUNIT_ASSERT(!startsWith(findReplaceTest, "findAnd"));
    CPPUNIT_ASSERT(startsWith(findReplaceTest, "findOr"));
    CPPUNIT_ASSERT(!startsWith(findReplaceTest, "findAnd"s));
    CPPUNIT_ASSERT(startsWith(findReplaceTest, "findOr"s));
    CPPUNIT_ASSERT(startsWith("test"s, "test"s));
    CPPUNIT_ASSERT(startsWith("test"s, "test"));
    CPPUNIT_ASSERT(!startsWith("test"s, "tests"s));
    CPPUNIT_ASSERT(!startsWith("test"s, "tests"));

    // endsWith()
    CPPUNIT_ASSERT(!endsWith(findReplaceTest, "AndReplace()"));
    CPPUNIT_ASSERT(endsWith(findReplaceTest, "OrReplace()"));
    CPPUNIT_ASSERT(!endsWith(findReplaceTest, "AndReplace()"s));
    CPPUNIT_ASSERT(endsWith(findReplaceTest, "OrReplace()"s));
    CPPUNIT_ASSERT(endsWith("test"s, "test"s));
    CPPUNIT_ASSERT(endsWith("test"s, "test"));
    CPPUNIT_ASSERT(!endsWith("test"s, " test"s));
    CPPUNIT_ASSERT(!endsWith("test"s, " test"));

    // containsSubstrings()
    CPPUNIT_ASSERT(containsSubstrings<string>("this string contains foo and bar", { "foo", "bar" }));
    CPPUNIT_ASSERT(!containsSubstrings<string>("this string contains foo and bar", { "bar", "foo" }));

    // truncateString()
    string truncateTest("foo  bar        ");
    truncateString(truncateTest, ' ');
    CPPUNIT_ASSERT_EQUAL("foo"s, truncateTest);

    // encodeBase64() / decodeBase64() with random data
    uniform_int_distribution<std::uint8_t> randomDistChar;
    std::uint8_t originalBase64Data[4047];
    for (std::uint8_t &c : originalBase64Data) {
        c = randomDistChar(m_randomEngine);
    }
    auto encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data));
    auto decodedBase64Data = decodeBase64(encodedBase64Data.data(), static_cast<std::uint32_t>(encodedBase64Data.size()));
    CPPUNIT_ASSERT(decodedBase64Data.second == sizeof(originalBase64Data));
    for (unsigned int i = 0; i < sizeof(originalBase64Data); ++i) {
        CPPUNIT_ASSERT(decodedBase64Data.first[i] == originalBase64Data[i]);
    }
    // test padding
    encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data) - 1);
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 1));
    CPPUNIT_ASSERT_NO_THROW(decodeBase64(encodedBase64Data.data(), static_cast<std::uint32_t>(encodedBase64Data.size())));
    encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data) - 2);
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 1));
    CPPUNIT_ASSERT_EQUAL('=', encodedBase64Data.at(encodedBase64Data.size() - 2));
    CPPUNIT_ASSERT_NO_THROW(decodeBase64(encodedBase64Data.data(), static_cast<std::uint32_t>(encodedBase64Data.size())));
    // test check for invalid size
    CPPUNIT_ASSERT_THROW(decodeBase64(encodedBase64Data.data(), 3), ConversionException);

    // dataSizeToString(), bitrateToString()
    CPPUNIT_ASSERT_EQUAL("512 bytes"s, dataSizeToString(512ull));
    CPPUNIT_ASSERT_EQUAL("2.50 KiB"s, dataSizeToString((2048ull + 512ull)));
    CPPUNIT_ASSERT_EQUAL("2.50 KiB (2560 byte)"s, dataSizeToString((2048ull + 512ull), true));
    CPPUNIT_ASSERT_EQUAL("2.50 MiB"s, dataSizeToString((2048ull + 512ull) * 1024ull));
    CPPUNIT_ASSERT_EQUAL("2.50 GiB"s, dataSizeToString((2048ull + 512ull) * 1024ull * 1024ull));
    CPPUNIT_ASSERT_EQUAL("2.50 TiB"s, dataSizeToString((2048ull + 512ull) * 1024ull * 1024ull * 1024ull));
    CPPUNIT_ASSERT_EQUAL("128 bit/s"s, bitrateToString(0.128, false));
    CPPUNIT_ASSERT_EQUAL("128 kbit/s"s, bitrateToString(128.0, false));
    CPPUNIT_ASSERT_EQUAL("128 Mbit/s"s, bitrateToString(128.0 * 1e3, false));
    CPPUNIT_ASSERT_EQUAL("128 Gbit/s"s, bitrateToString(128.0 * 1e6, false));
    CPPUNIT_ASSERT_EQUAL("16 byte/s"s, bitrateToString(0.128, true));
    CPPUNIT_ASSERT_EQUAL("16 KiB/s"s, bitrateToString(128.0, true));
    CPPUNIT_ASSERT_EQUAL("16 MiB/s"s, bitrateToString(128.0 * 1e3, true));
    CPPUNIT_ASSERT_EQUAL("16 GiB/s"s, bitrateToString(128.0 * 1e6, true));
}

/// \cond

struct ConvertibleToString {
    operator std::string() const;
};

struct StringThatDoesNotLikeToBeCopiedOrMoved : public std::string {
    explicit StringThatDoesNotLikeToBeCopiedOrMoved(const char *value)
        : std::string(value)
    {
    }
    [[noreturn]] StringThatDoesNotLikeToBeCopiedOrMoved(const StringThatDoesNotLikeToBeCopiedOrMoved &other)
        : std::string(other)
    {
        CPPUNIT_FAIL("attempt to copy string: " + other);
    }
    [[noreturn]] StringThatDoesNotLikeToBeCopiedOrMoved(StringThatDoesNotLikeToBeCopiedOrMoved &&other)
        : std::string(std::move(other))
    {
        CPPUNIT_FAIL("attempt to move string: " + other);
    }
};

/// \endcond

void ConversionTests::testStringBuilder()
{
    // check whether type traits work as expected
    static_assert(Helper::IsStringType<std::string, std::string>::value);
    static_assert(!Helper::IsStringType<std::string, std::wstring>::value);
    static_assert(Helper::IsStringType<std::wstring, std::wstring>::value);
    static_assert(Helper::IsStringViewType<std::string, std::string_view>::value);
    static_assert(!Helper::IsStringViewType<std::wstring, std::string_view>::value);
    static_assert(Helper::IsStringViewType<std::wstring, std::wstring_view>::value);
    static_assert(Helper::IsConvertibleToConstStringRef<std::string, ConvertibleToString>::value);
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
    static_assert(!Helper::IsConvertibleToConstStringRef<std::filesystem::path::string_type, std::filesystem::path>::value,
        "conversion via native() preferred");
#endif
    static_assert(
        !Helper::IsConvertibleToConstStringRef<std::string, std::string>::value, "yes, in this context this should not be considered convertible");
    static_assert(!Helper::IsConvertibleToConstStringRef<std::wstring, ConvertibleToString>::value);
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
    static_assert(Helper::IsConvertibleToConstStringRefViaNative<std::filesystem::path::string_type, std::filesystem::path>::value);
#endif
    static_assert(!Helper::IsConvertibleToConstStringRefViaNative<std::string, std::string>::value);

    // conversion of string-tuple to string (the actual string builder)
    const tuple<const char *, string, int, const char *> tuple("string1", "string2", 1234, "string3");
    CPPUNIT_ASSERT_EQUAL("string1string21234string3"s, tupleToString(tuple));
    CPPUNIT_ASSERT_EQUAL("foobarfoo2bar2"s, tupleToString("foo"s % "bar" % "foo2"s % "bar2"));
    CPPUNIT_ASSERT_EQUAL("v2.3.0"s, argsToString("v2.", 3, '.', 0));
    CPPUNIT_ASSERT_EQUAL("v2.3.0"s, argsToString('v', make_tuple(2, '.', 3, '.', 0)));
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
    if constexpr (std::is_same_v<std::filesystem::path::value_type, std::string::value_type>) {
        CPPUNIT_ASSERT_EQUAL("path: foo"s, argsToString("path: ", std::filesystem::path("foo")));
    }
#endif

    // construction of string-tuple and final conversion to string works
    CPPUNIT_ASSERT_EQUAL_MESSAGE("result can be passed to any function taking a std::string"s, "123456789"s, "12" % string("34") % '5' % 67 + "89");
    constexpr double velocityExample = 27.0;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("real-word example"s, "velocity: 27 km/h (7.5 m/s)"s,
        "velocity: " % numberToString(velocityExample) % " km/h (" % numberToString(velocityExample / 3.6) + " m/s)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "regular + operator still works (no problems with ambiguity)"s, "regular + still works"s, "regular"s + " + still works");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("using string_view", "foobar123"s, "foo"sv % "bar"sv + 123);

    // check that for the internal tuple construction no copies are made
    StringThatDoesNotLikeToBeCopiedOrMoved str(" happen ");
    const StringThatDoesNotLikeToBeCopiedOrMoved str2("for this");
    CPPUNIT_ASSERT_EQUAL("no copy/move should happen for this!"s,
        argsToString(StringThatDoesNotLikeToBeCopiedOrMoved("no copy/move should"), str, str2, StringThatDoesNotLikeToBeCopiedOrMoved("!")));
}
