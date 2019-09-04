#include "../misc/math.h"
#include "../tests/testutils.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace CppUtilities;
using namespace CppUtilities::Literals;

using namespace CPPUNIT_NS;

namespace CppUtilities {

static_assert(min(1, 2, 3) == 1, "min");
static_assert(min(3, 2, 1) == 1, "min");
static_assert(min(3, 4, 2, 1) == 1, "min");
static_assert(min(3, 4, -2, 2, 1) == -2, "min");
static_assert(max(1, 2, 3) == 3, "max");
static_assert(max(3, 2, 1) == 3, "max");
static_assert(max(3, 4, 2, 1) == 4, "max");
static_assert(max(3, -2, 4, 2, 1) == 4, "max");

} // namespace CppUtilities

/*!
 * \brief The MathTests class tests functions provided by misc/math.h.
 */
class MathTests : public TestFixture {
    CPPUNIT_TEST_SUITE(MathTests);
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

    void testDigitsum();
    void testFactorial();
    void testPowerModulo();
    void testInverseModulo();
    void testOrderModulo();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MathTests);

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
    CPPUNIT_ASSERT_EQUAL(25u, powerModulo(5u, 2u, 30u));
    CPPUNIT_ASSERT_EQUAL(5u, powerModulo(5u, 2u, 20u));
}

void MathTests::testInverseModulo()
{
    CPPUNIT_ASSERT_EQUAL(-12u, inverseModulo(2u, 25u));
    CPPUNIT_ASSERT_EQUAL(-8u, inverseModulo(3u, 25u));
}

void MathTests::testOrderModulo()
{
    CPPUNIT_ASSERT_EQUAL(20u, orderModulo(2u, 25u));
    CPPUNIT_ASSERT_EQUAL(5u, orderModulo(6u, 25u));
    CPPUNIT_ASSERT_EQUAL(0u, orderModulo(5u, 25u));
}
