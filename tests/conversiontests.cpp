#include "../conversion/binaryconversion.h"
#include "../conversion/stringconversion.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <random>
#include <sstream>
#include <functional>
#include <initializer_list>

using namespace std;
using namespace ConversionUtilities;

using namespace CPPUNIT_NS;

class ConversionTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(ConversionTests);
    CPPUNIT_TEST(testEndianness);
    CPPUNIT_TEST(testBinaryConversions);
    CPPUNIT_TEST(testSwapOrderFunctions);
    CPPUNIT_TEST(testStringConversions);
    CPPUNIT_TEST_SUITE_END();

public:
    ConversionTests();

    void setUp() {}
    void tearDown() {}

    void testEndianness();
    void testBinaryConversions();
    void testSwapOrderFunctions();
    void testStringConversions();

private:
    template<typename intType>
    void testConversion(const char *message, function<void (intType, char *)> vice, function<intType (const char *)> verca, intType min, intType max);

    char m_buff[8];
    random_device m_randomDevice;
    mt19937 m_randomEngine;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConversionTests);

ConversionTests::ConversionTests() :
    m_randomDevice(),
    m_randomEngine(m_randomDevice())
{}

/*!
 * \brief Tests whether macros for endianness are correct.
 */
void ConversionTests::testEndianness()
{
    union {
        uint32_t integer;
        char characters[4];
    } test = {0x01020304};
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

template<typename intType>
void ConversionTests::testConversion(const char *message, function<void (intType, char *)> vice, function<intType (const char *)> versa, intType min, intType max)
{
    const intType random = uniform_int_distribution<intType>(min, max)(m_randomEngine);
    stringstream msg;
    msg << message << '(' << hex << '0' << 'x' << random << ')';
    vice(random, m_buff);
    CPPUNIT_ASSERT_MESSAGE(msg.str(), versa(m_buff) == random);
}

#define TEST_TYPE(endianness, function) \
    decltype(endianness::function(m_buff))

#define TEST_CONVERSION(function, endianness) \
    testConversion<TEST_TYPE(endianness, function)>( \
        "testing " #function, \
        static_cast<void(*)(TEST_TYPE(endianness, function), char *)>(&endianness::getBytes), \
        endianness::function, \
        numeric_limits<TEST_TYPE(endianness, function)>::min(), \
        numeric_limits<TEST_TYPE(endianness, function)>::max() \
    )

#define TEST_BE_CONVERSION(function) \
    TEST_CONVERSION( \
        function, BE \
    )

#define TEST_LE_CONVERSION(function) \
    TEST_CONVERSION( \
        function, LE \
    )

#define TEST_CUSTOM_CONVERSION(vice, versa, endianness, min, max) \
    testConversion<TEST_TYPE(endianness, versa)>( \
        "testing " #versa, \
        static_cast<void(*)(TEST_TYPE(endianness, versa), char *)>(&endianness::vice), \
        endianness::versa, \
        min, max \
    )

/*!
 * \brief Tests most important binary conversions.
 *
 * Tests toUInt16(), ... toUInt64(), toInt16(), ... toInt64() and
 * the inverse getBytes() functions with random numbers.
 */
void ConversionTests::testBinaryConversions()
{
    // test to...() / getBytes() with random numbers
    for(byte b = 1; b < 100; ++b) {
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
 * \brief Tests most important string conversions.
 */
void ConversionTests::testStringConversions()
{
    // test stringToNumber() / numberToString() with random numbers
    uniform_int_distribution<int64> randomDistSigned(numeric_limits<int64>::min());
    uniform_int_distribution<uint64> randomDistUnsigned(0);
    for(byte b = 1; b < 100; ++b) {
        auto signedRandom = randomDistSigned(m_randomEngine);
        auto unsignedRandom = randomDistUnsigned(m_randomEngine);
        for(const auto base : initializer_list<byte>{2, 8, 10, 16}) {
            auto resultString = stringToNumber<uint64, string>(numberToString<uint64, string>(unsignedRandom, base), base);
            auto resultWideString = stringToNumber<uint64, wstring>(numberToString<uint64, wstring>(unsignedRandom, base), base);
            CPPUNIT_ASSERT(resultString == unsignedRandom);
            CPPUNIT_ASSERT(resultWideString == unsignedRandom);
        }
        for(const auto base : initializer_list<byte>{10}) {
            auto resultString = stringToNumber<int64, string>(numberToString<int64, string>(signedRandom, base), base);
            auto resultWideString = stringToNumber<int64, wstring>(numberToString<int64, wstring>(signedRandom, base), base);
            CPPUNIT_ASSERT(resultString == signedRandom);
            CPPUNIT_ASSERT(resultWideString == signedRandom);
        }
    }

    // test interpretIntegerAsString()
    CPPUNIT_ASSERT(interpretIntegerAsString<uint32>(0x54455354) == "TEST");

    // test splitString() / joinStrings()
    auto splitJoinTest = joinStrings(splitString<vector<string> >(",a,,ab,ABC,s", ",", EmptyPartsTreat::Keep), " ", false, "(", ")");
    CPPUNIT_ASSERT(splitJoinTest == "() (a) () (ab) (ABC) (s)");
    splitJoinTest = joinStrings(splitString<vector<string> >(",a,,ab,ABC,s", ",", EmptyPartsTreat::Keep), " ", true, "(", ")");
    CPPUNIT_ASSERT(splitJoinTest == "(a) (ab) (ABC) (s)");
    splitJoinTest = joinStrings(splitString<vector<string> >(",a,,ab,ABC,s", ",", EmptyPartsTreat::Omit), " ", false, "(", ")");
    CPPUNIT_ASSERT(splitJoinTest == "(a) (ab) (ABC) (s)");
    splitJoinTest = joinStrings(splitString<vector<string> >(",a,,ab,ABC,s", ",", EmptyPartsTreat::Merge), " ", false, "(", ")");
    CPPUNIT_ASSERT(splitJoinTest == "(a,ab) (ABC) (s)");

    // test findAndReplace()
    string findReplaceTest("findAndReplace()");
    findAndReplace<string>(findReplaceTest, "And", "Or");
    CPPUNIT_ASSERT(findReplaceTest == "findOrReplace()");

    // test startsWith()
    CPPUNIT_ASSERT(!startsWith<string>(findReplaceTest, "findAnd"));
    CPPUNIT_ASSERT(startsWith<string>(findReplaceTest, "findOr"));

    // test encodeBase64() / decodeBase64() with random data
    uniform_int_distribution<byte> randomDistChar;
    byte originalBase64Data[4047];
    for(byte &c : originalBase64Data) {
        c = randomDistChar(m_randomEngine);
    }
    const auto encodedBase64Data = encodeBase64(originalBase64Data, sizeof(originalBase64Data));
    auto decodedBase64Data = decodeBase64(encodedBase64Data.data(), encodedBase64Data.size());
    CPPUNIT_ASSERT(decodedBase64Data.second == sizeof(originalBase64Data));
    for(unsigned int i = 0; i < sizeof(originalBase64Data); ++i) {
        CPPUNIT_ASSERT(decodedBase64Data.first[i] == originalBase64Data[i]);
    }
}
