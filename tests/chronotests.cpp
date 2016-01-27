#include "../chrono/datetime.h"
#include "../chrono/timespan.h"
#include "../chrono/period.h"
#include "../conversion/conversionexception.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <iostream>

using namespace std;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

using namespace CPPUNIT_NS;

class ChronoTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(ChronoTests);
    CPPUNIT_TEST(testDateTime);
    CPPUNIT_TEST(testTimeSpan);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    void testDateTime();
    void testTimeSpan();
    void testOperators();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChronoTests);

/*!
 * \brief Tests most important DateTime features.
 */
void ChronoTests::testDateTime()
{
    // test year(), month(), ...
    auto test1 = DateTime::fromDateAndTime(2012, 2, 29, 15, 34, 20, 33.0);
    CPPUNIT_ASSERT(test1.year() == 2012);
    CPPUNIT_ASSERT(test1.month() == 2);
    CPPUNIT_ASSERT(test1.day() == 29);
    CPPUNIT_ASSERT(test1.minute() == 34);
    CPPUNIT_ASSERT(test1.second() == 20);
    CPPUNIT_ASSERT(test1.millisecond() == 33);
    CPPUNIT_ASSERT(test1.dayOfWeek() == DayOfWeek::Wednesday);
    CPPUNIT_ASSERT(test1.dayOfYear() == (31 + 29));
    CPPUNIT_ASSERT(test1.isLeapYear());
    CPPUNIT_ASSERT(test1.toString(DateTimeOutputFormat::DateTimeAndShortWeekday) == "Wed 2012-02-29 15:34:20.33");

    // test fromTimeStamp()
    auto test2 = DateTime::fromTimeStampGmt(1453840331);
    CPPUNIT_ASSERT(test2.toString(DateTimeOutputFormat::DateTimeAndShortWeekday) == "Tue 2016-01-26 20:32:11");

    // test whether ConversionException() is thrown when invalid values are specified
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2013, 2, 29, 15, 34, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 2, 29, 15, 61, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 4, 31, 15, 0, 20, 33), ConversionException);
    CPPUNIT_ASSERT_THROW(DateTime::fromDateAndTime(2012, 3, 31, 15, 0, 61, 33), ConversionException);
}

/*!
 * \brief Tests most important TimeSpan features.
 */
void ChronoTests::testTimeSpan()
{
    // test fromString(...), this should also test all other from...() methods and + operator
    auto test1 = TimeSpan::fromString("2:34:53:2.5");
    // test days(), hours(), ...
    CPPUNIT_ASSERT(test1.days() == 3);
    CPPUNIT_ASSERT(test1.hours() == 10);
    CPPUNIT_ASSERT(test1.minutes() == 53);
    CPPUNIT_ASSERT(test1.seconds() == 2);
    CPPUNIT_ASSERT(test1.milliseconds() == 500);
    CPPUNIT_ASSERT(test1.totalDays() > 3.0 && test1.totalDays() < 4.0);
    CPPUNIT_ASSERT(test1.totalHours() > (2 * 24 + 34) && test1.totalHours() < (2 * 24 + 35));
    CPPUNIT_ASSERT(test1.totalMinutes() > (2 * 24 * 60 + 34 * 60 + 53) && test1.totalHours() < (2 * 24 * 60 + 34 * 60 + 54));
    CPPUNIT_ASSERT(test1.toString(TimeSpanOutputFormat::WithMeasures, false) == "3 d 10 h 53 min 2 s 500 ms");

    // test whether ConversionException() is thrown when invalid values are specified
    CPPUNIT_ASSERT_THROW(TimeSpan::fromString("2:34a:53:32.5"), ConversionException);
}

/*!
 * \brief Tests operators of DateTime / TimeSpan.
 */
void ChronoTests::testOperators()
{
    auto dateTime = DateTime::fromDateAndTime(1999, 1, 5, 4, 16);
    CPPUNIT_ASSERT((dateTime + TimeSpan::fromDays(2)).day() == 7);
    CPPUNIT_ASSERT((dateTime + TimeSpan::fromHours(24)).day() == 6);
    CPPUNIT_ASSERT((dateTime + TimeSpan::fromHours(24) + TimeSpan::fromHours(-1)).hour() == 3);
    CPPUNIT_ASSERT((dateTime + TimeSpan::fromHours(24) - TimeSpan::fromMinutes(-1)).minute() == 17);
    dateTime += TimeSpan::fromDays(365);
    CPPUNIT_ASSERT(dateTime.year() == 2000);
    CPPUNIT_ASSERT(dateTime.day() == 5);
    CPPUNIT_ASSERT(Period(dateTime, dateTime + TimeSpan::fromDays(62)).months() == 2);
}
