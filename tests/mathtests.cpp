#include "../math/math.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace MathUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The MathTests class tests functions of the MathUtilities namespace.
 */
class MathTests : public TestFixture {
    CPPUNIT_TEST_SUITE(MathTests);
    CPPUNIT_TEST(testRandom);
    CPPUNIT_TEST(testDigitsum);
    CPPUNIT_TEST(testFactorial);
    CPPUNIT_TEST(testPowerModulo);
    CPPUNIT_TEST(testInverseModulo);
    CPPUNIT_TEST(testOrderModulo);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }
    void tearDown()
    {
    }

    void testRandom();
    void testDigitsum();
    void testFactorial();
    void testPowerModulo();
    void testInverseModulo();
    void testOrderModulo();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MathTests);

void MathTests::testRandom()
{
    CPPUNIT_ASSERT_EQUAL(6, random(5, 7));
}

void MathTests::testDigitsum()
{
    CPPUNIT_ASSERT_EQUAL(0, digitsum(0));
    CPPUNIT_ASSERT_EQUAL(7, digitsum(16));
    CPPUNIT_ASSERT_EQUAL(1, digitsum(16, 16));
}

void MathTests::testFactorial()
{
    CPPUNIT_ASSERT_EQUAL(6, factorial(3));
}

void MathTests::testPowerModulo()
{
    CPPUNIT_ASSERT_EQUAL(25ul, powerModulo(5, 2, 30));
    CPPUNIT_ASSERT_EQUAL(5ul, powerModulo(5, 2, 20));
}

void MathTests::testInverseModulo()
{
    CPPUNIT_ASSERT_EQUAL(-12l, inverseModulo(2, 25));
    CPPUNIT_ASSERT_EQUAL(-8l, inverseModulo(3, 25));
}

void MathTests::testOrderModulo()
{
    CPPUNIT_ASSERT_EQUAL(20ul, orderModulo(2, 25));
    CPPUNIT_ASSERT_EQUAL(5ul, orderModulo(6, 25));
    CPPUNIT_ASSERT_EQUAL(0ul, orderModulo(5, 25));
}
