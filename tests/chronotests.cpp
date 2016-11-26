#include "../chrono/datetime.h"
#include "../chrono/timespan.h"
#include "../chrono/period.h"
#include "../chrono/format.h"
#include "../conversion/conversionexception.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <iostream>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The ChronoTests class tests classes and methods of the ChronoUtilities namespace.
 */
class ChronoTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(ChronoTests);
    CPPUNIT_TEST(testDateTime);
    CPPUNIT_TEST(testTimeSpan);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST(testHashing);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    void testDateTime();
    void testTimeSpan();
    void testOperators();
    void testHashing();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChronoTests);

/*!
 * \brief Tests most important DateTime features.
 */
void ChronoTests::testDateTime()
{
    // test year(), month(), ...
    const auto test1 = DateTime::fromDateAndTime(2012, 2, 29, 15, 34, 20, 33.0);
    CPPUNIT_ASSERT_EQUAL(2012, test1.year());
    CPPUNIT_ASSERT_EQUAL(2, test1.month());
    CPPUNIT_ASSERT_EQUAL(29, test1.day());
    CPPUNIT_ASSERT_EQUAL(34, test1.minute());
    CPPUNIT_ASSERT_EQUAL(20, test1.second());
    CPPUNIT_ASSERT_EQUAL(33, test1.millisecond());
    CPPUNIT_ASSERT(test1.dayOfWeek() == DayOfWeek::Wednesday);
    CPPUNIT_ASSERT_EQUAL((31 + 29), test1.dayOfYear());
    CPPUNIT_ASSERT(test1.isLeapYear());
    CPPUNIT_ASSERT_EQUAL("Wed 2012-02-29 15:34:20.033"s, test1.toString(DateTimeOutputFormat::DateTimeAndShortWeekday));

    // test fromTimeStamp()
    const auto test2 = DateTime::fromTimeStampGmt(1453840331);
    CPPUNIT_ASSERT_EQUAL("Tue 2016-01-26 20:32:11"s, test2.toString(DateTimeOutputFormat::DateTimeAndShortWeekday));

    // test whether ConversionException() is thrown when invalid values are specified
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2013, 2, 29, 15, 34, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 2, 29, 15, 61, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 4, 31, 15, 0, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 3, 31, 15, 0, 61, 33), ConversionException);

    // test fromString()/toString()
    CPPUNIT_ASSERT_EQUAL(test1, DateTime::fromString("2012-02-29 15:34:20.033"));
    CPPUNIT_ASSERT_EQUAL("2012-02-29 15:34:20.033"s, test1.toString(DateTimeOutputFormat::DateAndTime, false));
    CPPUNIT_ASSERT_THROW(TimeSpan::fromString("2012-02-29 15:34:34:20.033"), ConversionException);
    const auto test3 = DateTime::fromIsoString("2016-08-29T21:32:31.125+02:00");
    CPPUNIT_ASSERT_EQUAL("2016-08-29T21:32:31.125+02:00"s, test3.first.toIsoString(test3.second));

    // test now() and exactNow() (or at least whether both behave the same)
#if defined(PLATFORM_UNIX)
    const auto delta = DateTime::gmtNow() - DateTime::exactGmtNow();
    CPPUNIT_ASSERT(delta < TimeSpan::fromSeconds(2) && delta > TimeSpan::fromSeconds(-2));
#endif
}

/*!
 * \brief Tests most important TimeSpan features.
 */
void ChronoTests::testTimeSpan()
{
    // test fromString(...), this should also test all other from...() methods and + operator
    auto test1 = TimeSpan::fromString("2:34:53:2.5");
    // test days(), hours(), ...
    CPPUNIT_ASSERT_EQUAL(3, test1.days());
    CPPUNIT_ASSERT_EQUAL(10, test1.hours());
    CPPUNIT_ASSERT_EQUAL(53, test1.minutes());
    CPPUNIT_ASSERT_EQUAL(2, test1.seconds());
    CPPUNIT_ASSERT_EQUAL(500, test1.milliseconds());
    CPPUNIT_ASSERT(test1.totalDays() > 3.0 && test1.totalDays() < 4.0);
    CPPUNIT_ASSERT(test1.totalHours() > (2 * 24 + 34) && test1.totalHours() < (2 * 24 + 35));
    CPPUNIT_ASSERT(test1.totalMinutes() > (2 * 24 * 60 + 34 * 60 + 53) && test1.totalHours() < (2 * 24 * 60 + 34 * 60 + 54));
    CPPUNIT_ASSERT_EQUAL("3 d 10 h 53 min 2 s 500 ms"s, test1.toString(TimeSpanOutputFormat::WithMeasures, false));

    // test whether ConversionException() is thrown when invalid values are specified
    CPPUNIT_ASSERT_THROW(TimeSpan::fromString("2:34a:53:32.5"), ConversionException);
}

/*!
 * \brief Tests operators of DateTime / TimeSpan.
 */
void ChronoTests::testOperators()
{
    auto dateTime = DateTime::fromDateAndTime(1999, 1, 5, 4, 16);
    CPPUNIT_ASSERT_EQUAL(7, (dateTime + TimeSpan::fromDays(2)).day());
    CPPUNIT_ASSERT_EQUAL(6, (dateTime + TimeSpan::fromHours(24)).day());
    CPPUNIT_ASSERT_EQUAL(3, (dateTime + TimeSpan::fromHours(24) + TimeSpan::fromHours(-1)).hour());
    CPPUNIT_ASSERT_EQUAL(17, (dateTime + TimeSpan::fromHours(24) - TimeSpan::fromMinutes(-1)).minute());
    dateTime += TimeSpan::fromDays(365);
    CPPUNIT_ASSERT_EQUAL(2000, dateTime.year());
    CPPUNIT_ASSERT_EQUAL(5, dateTime.day());
    CPPUNIT_ASSERT_EQUAL(2, Period(dateTime, dateTime + TimeSpan::fromDays(62)).months());
}

/*!
 * \brief Tests hashing DateTime / TimeSpan by using in a set.
 */
void ChronoTests::testHashing()
{
    set<DateTime> dateTimes;
    dateTimes.emplace(DateTime::fromDate(2500, 2, 1));
    dateTimes.emplace(DateTime::fromDate(2500, 2, 2));
    dateTimes.emplace(DateTime::fromDate(2500, 2, 1));
    CPPUNIT_ASSERT_EQUAL(2ul, dateTimes.size());

    set<TimeSpan> timeSpans;
    timeSpans.emplace(TimeSpan::fromDays(5));
    timeSpans.emplace(TimeSpan::fromDays(10));
    timeSpans.emplace(TimeSpan::fromDays(5));
    CPPUNIT_ASSERT_EQUAL(2ul, timeSpans.size());
}
