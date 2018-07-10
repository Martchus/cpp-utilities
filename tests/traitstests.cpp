#include "../misc/traits.h"
#include "../tests/testutils.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace Traits;

using namespace CPPUNIT_NS;

struct SomeStruct {
    string foo;
    int bar;
};

struct CountableStruct {
    int numberOfElements = 42;
    size_t size() const;
};

struct TestIncomplete;

static_assert(!Bool<false>::value, "Bool<false>");
static_assert(Bool<true>::value, "Bool<true>");
static_assert(!Not<Bool<true>>::value, "Not");
static_assert(!Any<Bool<false>, Bool<false>>::value, "Any: negative case");
static_assert(Any<Bool<true>, Bool<false>>::value, "Any: positive case");
static_assert(!All<Bool<true>, Bool<false>>::value, "All: negative case");
static_assert(All<Bool<true>, Bool<true>>::value, "All: positive case");
static_assert(!None<Bool<true>, Bool<false>>::value, "None: negative case");
static_assert(!None<Bool<true>, Bool<true>>::value, "None: negative case");
static_assert(None<Bool<false>, Bool<false>>::value, "None: positive case");

static_assert(!IsSpecializationOf<string, basic_stringbuf>::value, "IsSpecializationOf: negative case");
static_assert(IsSpecializationOf<string, basic_string>::value, "IsSpecializationOf: positive case");
static_assert(!IsSpecializingAnyOf<string, basic_stringbuf, vector>::value, "IsSpecializingAnyOf: negative case");
static_assert(!IsSpecializingAnyOf<string, basic_stringbuf, list, vector>::value, "IsSpecializingAnyOf: negative case");
static_assert(IsSpecializingAnyOf<string, basic_stringbuf, basic_string, vector>::value, "IsSpecializingAnyOf: positive case");
static_assert(IsSpecializingAnyOf<string, basic_stringbuf, vector, basic_string>::value, "IsSpecializingAnyOf: positive case");
static_assert(IsSpecializingAnyOf<string, basic_string>::value, "IsSpecializingAnyOf: positive case");

static_assert(IsAnyOf<string, string, int, bool>::value, "IsAnyOf: positive case");
static_assert(IsAnyOf<int, string, int, bool>::value, "IsAnyOf: positive case");
static_assert(IsAnyOf<bool, string, int, bool>::value, "IsAnyOf: positive case");
static_assert(!IsAnyOf<unsigned int, string, int, bool>::value, "IsAnyOf: negative case");
static_assert(!IsNoneOf<string, string, int, bool>::value, "IsNoneOf: negative case");
static_assert(!IsNoneOf<int, string, int, bool>::value, "IsNoneOf: negative case");
static_assert(!IsNoneOf<bool, string, int, bool>::value, "IsNoneOf: negative case");
static_assert(IsNoneOf<unsigned int, string, int, bool>::value, "IsNoneOf: positive case");

static_assert(!IsDereferencable<string>::value, "IsDereferencable: negative case");
static_assert(!IsDereferencable<int>::value, "IsDereferencable: negative case");
static_assert(IsDereferencable<string *>::value, "IsDereferencable: positive case");
static_assert(IsDereferencable<int *>::value, "IsDereferencable: positive case");
static_assert(IsDereferencable<unique_ptr<string>>::value, "IsDereferencable: positive case");
static_assert(IsDereferencable<shared_ptr<string>>::value, "IsDereferencable: positive case");
static_assert(!IsDereferencable<weak_ptr<string>>::value, "IsDereferencable: positive case");

static_assert(!IsIteratable<int>::value, "IsIterator: negative case");
static_assert(!IsIteratable<SomeStruct>::value, "IsIterator: negative case");
static_assert(IsIteratable<string>::value, "IsIterator: positive case");
static_assert(IsIteratable<vector<int>>::value, "IsIterator: positive case");
static_assert(IsIteratable<list<string>>::value, "IsIterator: positive case");
static_assert(IsIteratable<map<string, string>>::value, "IsIterator: positive case");
static_assert(IsIteratable<initializer_list<double>>::value, "IsIterator: positive case");
static_assert(!HasSize<SomeStruct>::value, "HasSize: negative case");
static_assert(!HasSize<forward_list<SomeStruct>>::value, "HasSize: negative case");
static_assert(HasSize<vector<SomeStruct>>::value, "HasSize: positive case");
static_assert(HasSize<string>::value, "HasSize: positive case");
static_assert(HasSize<CountableStruct>::value, "HasSize: positive case");
static_assert(!IsReservable<list<SomeStruct>>::value, "HasSize: negative case");
static_assert(IsReservable<vector<SomeStruct>>::value, "HasSize: positive case");

static_assert(!IsCString<string>::value, "IsCString: negative case");
static_assert(!IsCString<int[]>::value, "IsCString: negative case");
static_assert(!IsCString<int *>::value, "IsCString: negative case");
static_assert(IsCString<char[]>::value, "IsCString: positive case");
static_assert(IsCString<char *>::value, "IsCString: positive case");
static_assert(IsCString<const char *>::value, "IsCString: positive case");
static_assert(!IsString<int *>::value, "IsString: negative case");
static_assert(!IsString<stringstream>::value, "IsString: negative case");
static_assert(IsString<const char *>::value, "IsString: positive case");
static_assert(IsString<string>::value, "IsCString: positive case");
static_assert(IsString<u16string>::value, "IsCString: positive case");

static_assert(!IsComplete<TestIncomplete>::value, "IsComplete: negative case");
static_assert(IsComplete<CountableStruct>::value, "IsComplete: positive case");

constexpr int i = 5;
constexpr CountableStruct someStruct{};
static_assert(dereferenceMaybe(&i) == 5, "int* dereferenced");
static_assert(dereferenceMaybe(i) == 5, "int not dereferenced");
static_assert(dereferenceMaybe(&someStruct).numberOfElements == 42, "CountableStruct* dereferenced");
static_assert(dereferenceMaybe(someStruct).numberOfElements == 42, "CountableStruct not dereferenced");

/*!
 * \brief The TraitsTest class tests parts of the Traits namespace which can not be evaluated at compile-time.
 */
class TraitsTest : public TestFixture {
    CPPUNIT_TEST_SUITE(TraitsTest);
    CPPUNIT_TEST(testDereferenceMaybe);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }
    void tearDown()
    {
    }

    void testDereferenceMaybe();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TraitsTest);

/*!
 * \brief Tests whether a smart pointer to a string can be treated like a normal string through the use of dereferenceMaybe().
 */
void TraitsTest::testDereferenceMaybe()
{
    const auto someString = "foo"s;
    const auto someSmartPointer = make_unique<string>("foo");
    CPPUNIT_ASSERT_EQUAL("foo"s, dereferenceMaybe(someString));
    CPPUNIT_ASSERT_EQUAL("foo"s, dereferenceMaybe(someSmartPointer));
}
