#include "../misc/traits.h"

#include <forward_list>
#include <list>
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace Traits;

struct SomeStruct {
    string foo;
    int bar;
};

struct CountableStruct {
    int numberOfElements;
    size_t size() const;
};

static_assert(!Bool<false>::value, "Bool<false>");
static_assert(Bool<true>::value, "Bool<true>");
static_assert(!Not<Bool<true>>::value, "Not");
static_assert(!Any<Bool<false>, Bool<false>>::value, "Any: negative case");
static_assert(Any<Bool<true>, Bool<false>>::value, "Any: positive case");
static_assert(!All<Bool<true>, Bool<false>>::value, "All: negative case");
static_assert(All<Bool<true>, Bool<true>>::value, "All: positive case");

static_assert(!IsSpecializationOf<string, basic_stringbuf>::value, "IsSpecializationOf: negative case");
static_assert(IsSpecializationOf<string, basic_string>::value, "IsSpecializationOf: positive case");

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
