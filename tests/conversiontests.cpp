#include "../conversion/binaryconversion.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <random>
#include <sstream>
#include <functional>

using namespace std;
using namespace ConversionUtilities;

using namespace CPPUNIT_NS;

class ConversionTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(ConversionTests);
    CPPUNIT_TEST(testEndianness);
    CPPUNIT_TEST(testBinaryConversions);
    CPPUNIT_TEST_SUITE_END();

public:
    ConversionTests();

    void setUp() {}
    void tearDown() {}

    void testEndianness();
    void testBinaryConversions();

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
    CPPUNIT_ASSERT_MESSAGE("Byte order assumption (big-endian) is wrong", test.characters[0] == 0x01);
#elif defined(CONVERSION_UTILITIES_BYTE_ORDER_LITTLE_ENDIAN)
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
    const intType result = versa(m_buff);
    CPPUNIT_ASSERT_MESSAGE(msg.str(), result == random);
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
 * \brief Tests some binary conversions.
 *
 * Tests toUInt16(), ... toUInt64(), toInt16(), ... toInt64() and
 * the inverse getBytes() functions.
 */
void ConversionTests::testBinaryConversions()
{
    for(byte b = 0; b < 100; ++b) {
        TEST_BE_CONVERSION(toUInt16);
        TEST_CUSTOM_CONVERSION(getBytes24, toUInt24, BE, 0, 0xFFFFFF);
        TEST_BE_CONVERSION(toUInt32);
        TEST_BE_CONVERSION(toUInt64);
        TEST_LE_CONVERSION(toUInt16);
        TEST_CUSTOM_CONVERSION(getBytes24, toUInt24, LE, 0, 0xFFFFFF);
        TEST_LE_CONVERSION(toUInt32);
        TEST_LE_CONVERSION(toUInt64);
        TEST_BE_CONVERSION(toInt16);
        TEST_BE_CONVERSION(toInt32);
        TEST_BE_CONVERSION(toInt64);
        TEST_LE_CONVERSION(toInt16);
        TEST_LE_CONVERSION(toInt32);
        TEST_LE_CONVERSION(toInt64);
    }

}
