#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

using namespace std;

using namespace CPPUNIT_NS;

class IoTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(IoTests);
    CPPUNIT_TEST(testFailure);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    void testFailure();
};

CPPUNIT_TEST_SUITE_REGISTRATION(IoTests);

/*!
 * \brief Tests for GCC Bug 66145.
 * \sa https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
 */
void IoTests::testFailure()
{
    fstream stream;
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    CPPUNIT_ASSERT_THROW(stream.open("path/to/file/which/does/not/exist", ios_base::in), ios_base::failure);
}
