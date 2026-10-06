#include "../misc/regex.h"

#ifdef CPP_UTILITIES_USE_PCRE2

#include "../tests/testutils.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace std;
using namespace CppUtilities;
using namespace CPPUNIT_NS;

/*!
 * \brief The RegexTests class tests classes and functions provided by regex/regex.h.
 */
class RegexTests : public TestFixture {
    CPPUNIT_TEST_SUITE(RegexTests);
    CPPUNIT_TEST(testCompilation);
    CPPUNIT_TEST(testMatching);
    CPPUNIT_TEST(testCaptures);
    CPPUNIT_TEST(testOptions);
    CPPUNIT_TEST(testMatchAll);
    CPPUNIT_TEST(testLifetimeAndResourceManagement);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }
    void tearDown()
    {
    }

    void testCompilation();
    void testMatching();
    void testCaptures();
    void testOptions();
    void testMatchAll();
    void testLifetimeAndResourceManagement();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegexTests);

void RegexTests::testCompilation()
{
    // default constructor creates an uncompiled regex
    auto defaultRe = Regex();
    CPPUNIT_ASSERT(!defaultRe.isValid());
    CPPUNIT_ASSERT(!defaultRe);
    CPPUNIT_ASSERT(defaultRe.pattern().empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(RegexCompileOptions::None), static_cast<std::uint32_t>(defaultRe.compileOptions()));

    // valid compilation
    auto validRe = Regex("^[a-z0-9_]+$");
    CPPUNIT_ASSERT(validRe.isValid());
    CPPUNIT_ASSERT(static_cast<bool>(validRe));
    CPPUNIT_ASSERT_EQUAL(std::string_view("^[a-z0-9_]+$"), validRe.pattern());

    // recompile with compile()
    validRe.compile(R"(\d+)");
    CPPUNIT_ASSERT(validRe.isValid());
    CPPUNIT_ASSERT_EQUAL(std::string_view(R"(\d+)"), validRe.pattern());

    // invalid pattern throws RegexException
    try {
        auto invalidRe = Regex("[a-z");
        CPPUNIT_FAIL("Expected RegexException was not thrown");
    } catch (const RegexException &e) {
        CPPUNIT_ASSERT(e.offset() > 0);
        CPPUNIT_ASSERT(std::string_view(e.what()).size() > 0);
    }

    // compile invalid pattern on existing regex
    CPPUNIT_ASSERT_THROW(validRe.compile("(unclosed"), RegexException);

    // clear resets regex to uncompiled state
    validRe.clear();
    CPPUNIT_ASSERT(!validRe.isValid());
    CPPUNIT_ASSERT(validRe.pattern().empty());

    // setPattern and setCompileOptions
    auto optionRe = Regex();
    optionRe.setCompileOptions(RegexCompileOptions::CaseInsensitive);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(RegexCompileOptions::CaseInsensitive), static_cast<std::uint32_t>(optionRe.compileOptions()));
    optionRe.setPattern("abc");
    CPPUNIT_ASSERT(optionRe.isValid());
    CPPUNIT_ASSERT(optionRe.isMatch("ABC"));
}

void RegexTests::testMatching()
{
    auto re = Regex("^[a-z]+$");

    // isMatch
    CPPUNIT_ASSERT(re.isMatch("hello"));
    CPPUNIT_ASSERT(!re.isMatch("HELLO"));
    CPPUNIT_ASSERT(!re.isMatch("hello world"));
    CPPUNIT_ASSERT(!re.isMatch("123"));

    // match
    const auto match1 = re.match("hello");
    CPPUNIT_ASSERT(match1.hasMatch());
    CPPUNIT_ASSERT(static_cast<bool>(match1));
    CPPUNIT_ASSERT_EQUAL(std::string_view("hello"), match1.captured());
    CPPUNIT_ASSERT_EQUAL(std::string_view("hello"), match1[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), match1.capturedString());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), match1.captureCount());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match1.capturedStart());
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), match1.capturedEnd());
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), match1.capturedLength());
    CPPUNIT_ASSERT_EQUAL(std::string_view("hello"), match1.subject());

    // failed match
    const auto match2 = re.match("HELLO");
    CPPUNIT_ASSERT(!match2.hasMatch());
    CPPUNIT_ASSERT(!match2);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match2.captureCount());
    CPPUNIT_ASSERT(match2.captured().empty());
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, match2.capturedStart());
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, match2.capturedEnd());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match2.capturedLength());

    // matching with offset
    auto wordRe = Regex("[a-z]+");
    const auto matchOffset = wordRe.match("foo bar baz", 4);
    CPPUNIT_ASSERT(matchOffset.hasMatch());
    CPPUNIT_ASSERT_EQUAL(std::string_view("bar"), matchOffset.captured());
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), matchOffset.capturedStart());
    CPPUNIT_ASSERT_EQUAL(std::size_t(7), matchOffset.capturedEnd());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), matchOffset.capturedLength());

    // offset out of bounds returns no match
    const auto matchOutOfBounds = wordRe.match("foo", 10);
    CPPUNIT_ASSERT(!matchOutOfBounds.hasMatch());

    // matching on invalid regex returns no match
    auto invalidRe = Regex();
    CPPUNIT_ASSERT(!invalidRe.isMatch("foo"));
    CPPUNIT_ASSERT(!invalidRe.match("foo").hasMatch());
}

void RegexTests::testCaptures()
{
    // multiple capturing groups
    auto re = Regex(R"((\w+)\s*=\s*(\d+))");
    const auto match = re.match("key = 12345");
    CPPUNIT_ASSERT(match.hasMatch());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), match.captureCount());

    // group 0 (full match)
    CPPUNIT_ASSERT(match.hasCaptured(0));
    CPPUNIT_ASSERT_EQUAL(std::string_view("key = 12345"), match.captured(0));
    CPPUNIT_ASSERT_EQUAL(std::string_view("key = 12345"), match[0]);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match.capturedStart(0));
    CPPUNIT_ASSERT_EQUAL(std::size_t(11), match.capturedEnd(0));
    CPPUNIT_ASSERT_EQUAL(std::size_t(11), match.capturedLength(0));

    // group 1
    CPPUNIT_ASSERT(match.hasCaptured(1));
    CPPUNIT_ASSERT_EQUAL(std::string_view("key"), match.captured(1));
    CPPUNIT_ASSERT_EQUAL(std::string_view("key"), match[1]);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match.capturedStart(1));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), match.capturedEnd(1));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), match.capturedLength(1));

    // group 2
    CPPUNIT_ASSERT(match.hasCaptured(2));
    CPPUNIT_ASSERT_EQUAL(std::string_view("12345"), match.captured(2));
    CPPUNIT_ASSERT_EQUAL(std::string_view("12345"), match[2]);
    CPPUNIT_ASSERT_EQUAL(std::size_t(6), match.capturedStart(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(11), match.capturedEnd(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), match.capturedLength(2));

    // out-of-bounds group index
    CPPUNIT_ASSERT(!match.hasCaptured(3));
    CPPUNIT_ASSERT(match.captured(3).empty());
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, match.capturedStart(3));
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, match.capturedEnd(3));
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), match.capturedLength(3));

    // non-participating capture group
    auto altRe = Regex("(cat)|(dog)");
    const auto matchAlt = altRe.match("dog");
    CPPUNIT_ASSERT(matchAlt.hasMatch());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), matchAlt.captureCount());
    CPPUNIT_ASSERT_EQUAL(std::string_view("dog"), matchAlt.captured(0));
    CPPUNIT_ASSERT(!matchAlt.hasCaptured(1));
    CPPUNIT_ASSERT(matchAlt.captured(1).empty());
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, matchAlt.capturedStart(1));
    CPPUNIT_ASSERT_EQUAL(std::string_view::npos, matchAlt.capturedEnd(1));
    CPPUNIT_ASSERT(matchAlt.hasCaptured(2));
    CPPUNIT_ASSERT_EQUAL(std::string_view("dog"), matchAlt.captured(2));
}

void RegexTests::testOptions()
{
    // CaseInsensitive
    auto reCase = Regex("hello", RegexCompileOptions::CaseInsensitive);
    CPPUNIT_ASSERT(reCase.isMatch("HELLO"));
    CPPUNIT_ASSERT(reCase.isMatch("hElLo"));

    // Multiline
    auto reMulti = Regex("^line2", RegexCompileOptions::Multiline);
    CPPUNIT_ASSERT(reMulti.isMatch("line1\nline2\nline3"));

    // DotAll
    auto reDot = Regex("start.*end", RegexCompileOptions::DotAll);
    CPPUNIT_ASSERT(reDot.isMatch("start\nmiddle\nend"));

    // Extended
    auto reExt = Regex("a b c # comment\n d", RegexCompileOptions::Extended);
    CPPUNIT_ASSERT(reExt.isMatch("abcd"));

    // Ungreedy
    auto reUngreedy = Regex("<.*>", RegexCompileOptions::Ungreedy);
    const auto matchUngreedy = reUngreedy.match("<a><b>");
    CPPUNIT_ASSERT(matchUngreedy.hasMatch());
    CPPUNIT_ASSERT_EQUAL(std::string_view("<a>"), matchUngreedy.captured());

    // Utf
    auto reUtf = Regex(R"(^\w+$)", RegexCompileOptions::Utf);
    CPPUNIT_ASSERT(reUtf.isMatch("täst"));

    // NotBeginningOfLine
    auto reBol = Regex("^start");
    CPPUNIT_ASSERT(reBol.isMatch("start"));
    CPPUNIT_ASSERT(!reBol.isMatch("start", 0, RegexMatchOptions::NotBeginningOfLine));

    // NotEndOfLine
    auto reEol = Regex("end$");
    CPPUNIT_ASSERT(reEol.isMatch("end"));
    CPPUNIT_ASSERT(!reEol.isMatch("end", 0, RegexMatchOptions::NotEndOfLine));

    // NotEmpty
    auto reEmpty = Regex("a*");
    CPPUNIT_ASSERT(reEmpty.isMatch("b"));
    CPPUNIT_ASSERT(!reEmpty.isMatch("b", 0, RegexMatchOptions::NotEmpty));
}

void RegexTests::testMatchAll()
{
    auto re = Regex(R"(\d+)");
    const auto matches = re.matchAll("order 12 has 34 items and 56 boxes");
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), matches.size());
    CPPUNIT_ASSERT_EQUAL(std::string_view("12"), matches[0].captured());
    CPPUNIT_ASSERT_EQUAL(std::string_view("34"), matches[1].captured());
    CPPUNIT_ASSERT_EQUAL(std::string_view("56"), matches[2].captured());

    // matchAll on invalid regex
    auto invalidRe = Regex();
    CPPUNIT_ASSERT(invalidRe.matchAll("test").empty());
}

void RegexTests::testLifetimeAndResourceManagement()
{
    // copy construction
    auto re1 = Regex(R"([a-z]+)", RegexCompileOptions::CaseInsensitive);
    auto re2 = re1;
    CPPUNIT_ASSERT(re2.isValid());
    CPPUNIT_ASSERT_EQUAL(re1.pattern(), re2.pattern());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(re1.compileOptions()), static_cast<std::uint32_t>(re2.compileOptions()));
    CPPUNIT_ASSERT(re2.isMatch("ABC"));

    // copy assignment
    auto re3 = Regex();
    re3 = re1;
    CPPUNIT_ASSERT(re3.isValid());
    CPPUNIT_ASSERT(re3.isMatch("ABC"));

    // self assignment
    re3 = re3;
    CPPUNIT_ASSERT(re3.isValid());
    CPPUNIT_ASSERT(re3.isMatch("ABC"));

    // move construction
    auto reMoved = std::move(re1);
    CPPUNIT_ASSERT(reMoved.isValid());
    CPPUNIT_ASSERT(reMoved.isMatch("ABC"));
    CPPUNIT_ASSERT(!re1.isValid());

    // move assignment
    auto reTarget = Regex();
    reTarget = std::move(reMoved);
    CPPUNIT_ASSERT(reTarget.isValid());
    CPPUNIT_ASSERT(reTarget.isMatch("ABC"));
    CPPUNIT_ASSERT(!reMoved.isValid());

    // resource cleanup when going out of scope
    {
        auto scopedRe = Regex(R"(\d+)");
        CPPUNIT_ASSERT(scopedRe.isValid());
        const auto m = scopedRe.match("42");
        CPPUNIT_ASSERT(m.hasMatch());
    }
}

#endif // CPP_UTILITIES_USE_PCRE2
