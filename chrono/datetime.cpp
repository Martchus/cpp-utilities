#include "./datetime.h"

#include "../conversion/stringconversion.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>

using namespace std;
using namespace ChronoUtilities;
using namespace ConversionUtilities;

const int DateTime::m_daysPerYear = 365;
const int DateTime::m_daysPer4Years = 1461;
const int DateTime::m_daysPer100Years = 36524;
const int DateTime::m_daysPer400Years = 146097;
const int DateTime::m_daysTo1601 = 584388;
const int DateTime::m_daysTo1899 = 693593;
const int DateTime::m_daysTo10000 = 3652059;
const int DateTime::m_daysToMonth365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
const int DateTime::m_daysToMonth366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
const int DateTime::m_daysInMonth365[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int DateTime::m_daysInMonth366[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

template<typename num1, typename num2, typename num3>
inline bool inRangeInclMax(num1 val, num2 min, num3 max)
{
    return (val) >= (min) && (val) <= (max);
}

template<typename num1, typename num2, typename num3>
inline bool inRangeExclMax(num1 val, num2 min, num3 max)
{
    return (val) >= (min) && (val) <  (max);
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the local time.
 */
DateTime DateTime::now()
{
    return DateTime::fromTimeStamp(time(nullptr));
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the GMT time.
 */
DateTime DateTime::gmtNow()
{
    return DateTime::fromTimeStampGmt(time(nullptr));
}

/*!
 * \class ChronoUtilities::DateTime
 * \brief Represents an instant in time, typically expressed as a date and time of day.
 * \remarks
 *  - Time values are measured in 100-nanosecond units called ticks,
 *    and a particular date is the number of ticks since 12:00 midnight, January 1,
 *    0001 A.D. (C.E.) in the GregorianCalendar calendar (excluding ticks that would
 *    be added by leap seconds).
 *  - There is no time zone information associated. Hence different time zones are
 *    not taken into account when comparing two instances. For instance the
 *    expression (DateTime::now() - DateTime::gmtNow()) returns one hour in Germany (instead of zero).
 */

/*!
 * \brief Constructs a new DateTime object with the local time from the specified \a timeStamp.
 */
DateTime DateTime::fromTimeStamp(time_t timeStamp)
{
    if(timeStamp) {
        struct tm *timeinfo = localtime(&timeStamp);
        return DateTime::fromDateAndTime(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec < 60 ? timeinfo->tm_sec : 59, 0);
    } else {
        return DateTime();
    }
}

/*!
 * \brief Constructs a new DateTime object with the GMT time from the specified \a timeStamp.
 */
DateTime DateTime::fromTimeStampGmt(time_t timeStamp)
{
    if(timeStamp) {
        struct tm *timeinfo = gmtime(&timeStamp);
        return DateTime::fromDateAndTime(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec < 60 ? timeinfo->tm_sec : 59, 0);
    } else {
        return DateTime();
    }
}

/*!
 * \brief Parses the given std::string \a str as DateTime.
 */
DateTime DateTime::fromString(const string &str)
{
    int values[7] = {0};
    int *i = values;
    for(const auto &c : str) {
        if(c >= '1' || c <= '0') {
            *i *= 10;
            *i += c - '1';
        } else if((c == '-' || c == ':' || c == '/') || (c == '.' && (i == values + 5))) {
            ++i;
        } else {
            throw ConversionException(string("string contains unexpected character ") + c);
        }
    }
    return DateTime::fromDateAndTime(values[0], values[1], values[2], values[3], values[4], values[5], 100.0 * values[6]);
}

/*!
 * \brief Converts the value of the current DateTime object to its equivalent std::string representation
 *        according the given \a format.
 *
 * If \a noMilliseconds is true the date will be rounded to full seconds.
 */
string DateTime::toString(DateTimeOutputFormat format, bool noMilliseconds) const
{
    string result;
    toString(result, format, noMilliseconds);
    return result;
}

/*!
 * \brief Converts the value of the current DateTime object to its equivalent std::string representation
 *        according the given \a format.
 *
 * If \a noMilliseconds is true the date will be rounded to full seconds.
 */
void DateTime::toString(string &result, DateTimeOutputFormat format, bool noMilliseconds) const
{
    stringstream s(stringstream::in | stringstream::out);
    s << setfill('0');
    if(format == DateTimeOutputFormat::DateTimeAndWeekday
            || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << printDayOfWeek(dayOfWeek(), format == DateTimeOutputFormat::DateTimeAndShortWeekday) << " ";
    if(format == DateTimeOutputFormat::DateOnly
            || format == DateTimeOutputFormat::DateAndTime
            || format == DateTimeOutputFormat::DateTimeAndWeekday
            || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << setw(4) << year() << "-" << setw(2) << month() << "-" << setw(2) << day();
    if(format == DateTimeOutputFormat::DateAndTime
            || format == DateTimeOutputFormat::DateTimeAndWeekday
            || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << " ";
    if(format == DateTimeOutputFormat::TimeOnly
            || format == DateTimeOutputFormat::DateAndTime
            || format == DateTimeOutputFormat::DateTimeAndWeekday
            || format == DateTimeOutputFormat::DateTimeAndShortWeekday) {
        s << setw(2) << hour() << ":" << setw(2) << minute() << ":" << setw(2) << second();
        int ms = millisecond();
        if(!noMilliseconds && ms > 0) {
            s << "." << ms;
        }
    }
    result = s.str();
}

/*!
 * \brief Returns the string representation as C-style string for the given day of week.
 *
 * If \a abbreviation is true, only the first three letters of the string will
 * be returned.
 * \sa DayOfWeek
 */
const char *DateTime::printDayOfWeek(DayOfWeek dayOfWeek, bool abbreviation)
{
    if(abbreviation) {
        switch(dayOfWeek) {
        case DayOfWeek::Monday:
            return "Mon";
        case DayOfWeek::Tuesday:
            return "Tue";
        case DayOfWeek::Wednesday:
            return "Wed";
        case DayOfWeek::Thursday:
            return "Thu";
        case DayOfWeek::Friday:
            return "Fri";
        case DayOfWeek::Saturday:
            return "Sat";
        case DayOfWeek::Sunday:
            return "Sun";
        }
    } else {
        switch(dayOfWeek) {
        case DayOfWeek::Monday:
            return "Monday";
        case DayOfWeek::Tuesday:
            return "Tuesday";
        case DayOfWeek::Wednesday:
            return "Wednesday";
        case DayOfWeek::Thursday:
            return "Thursday";
        case DayOfWeek::Friday:
            return "Friday";
        case DayOfWeek::Saturday:
            return "Saturday";
        case DayOfWeek::Sunday:
            return "Sunday";
        }
    }
    return "";
}

/*!
 * \brief Converts the given date expressed in \a year, \a month and \a day to ticks.
 */
uint64 DateTime::dateToTicks(int year, int month, int day)
{
    if(inRangeInclMax(year, 1, 9999)) {
        if(inRangeInclMax(month, 1, 12)) {
            const int *daysToMonth = isLeapYear(year) ? m_daysToMonth366 : m_daysToMonth365;
            int passedMonth = month - 1;
            if(inRangeInclMax(day, 1, daysToMonth[month] - daysToMonth[passedMonth])) {
                int passedYears = year - 1;
                int passedDays = day - 1;
                return (passedYears * m_daysPerYear + passedYears / 4 - passedYears / 100 + passedYears / 400 + daysToMonth[passedMonth] + passedDays) * TimeSpan::m_ticksPerDay;
            } else {
                throw ConversionException("day is out of range");
            }
        } else {
            throw ConversionException("month is out of range");
        }
    } else {
        throw ConversionException("year is out of range");
    }
    return 0;
}

/*!
 * \brief Converts the given time expressed in \a hour, \a minute, \a second and \a millisecond to ticks.
 */
uint64 DateTime::timeToTicks(int hour, int minute, int second, double millisecond)
{
    if(!inRangeExclMax(hour, 0, 24)) {
        throw ConversionException("hour is out of range");
    }
    if(!inRangeExclMax(minute, 0, 60)) {
        throw ConversionException("minute is out of range");
    }
    if(!inRangeExclMax(second, 0, 60)) {
        throw ConversionException("second is out of range");
    }
    if(!inRangeExclMax(millisecond, 0.0, 1000.0)) {
        throw ConversionException("millisecond is out of range");
    }
    return (hour * TimeSpan::m_ticksPerHour) + (minute * TimeSpan::m_ticksPerMinute) + (second * TimeSpan::m_ticksPerSecond) + (uint64)(millisecond * (double)TimeSpan::m_ticksPerMillisecond);
}

/*!
 * \brief Returns the specified date part.
 * \sa DatePart
 */
int DateTime::getDatePart(DatePart part) const
{
    int fullDays = m_ticks / TimeSpan::m_ticksPerDay;
    int full400YearBlocks = fullDays / m_daysPer400Years;
    int daysMinusFull400YearBlocks = fullDays - full400YearBlocks * m_daysPer400Years;
    int full100YearBlocks = daysMinusFull400YearBlocks / m_daysPer100Years;
    if(full100YearBlocks == 4) {
        full100YearBlocks = 3;
    }
    int daysMinusFull100YearBlocks = daysMinusFull400YearBlocks - full100YearBlocks * m_daysPer100Years;
    int full4YearBlocks = daysMinusFull100YearBlocks / m_daysPer4Years;
    int daysMinusFull4YearBlocks = daysMinusFull100YearBlocks - full4YearBlocks * m_daysPer4Years;
    int full1YearBlocks = daysMinusFull4YearBlocks / m_daysPerYear;
    if(full1YearBlocks == 4) {
        full1YearBlocks = 3;
    }
    if(part == DatePart::Year) {
        return full400YearBlocks * 400 + full100YearBlocks * 100 + full4YearBlocks * 4 + full1YearBlocks + 1;
    }
    int restDays = daysMinusFull4YearBlocks - full1YearBlocks * m_daysPerYear;
    if(part == DatePart::DayOfYear) { // day
        return restDays + 1;
    }
    const int *daysToMonth = (full1YearBlocks == 3 && (full4YearBlocks != 24 || full100YearBlocks == 3)) ? m_daysToMonth366 : m_daysToMonth365;
    int month = 1;
    while(restDays >= daysToMonth[month]) {
        ++month;
    }
    if(part == DatePart::Month) {
        return month;
    } else if(part == DatePart::Day) {
        return restDays - daysToMonth[month - 1] + 1;
    }
    return 0;
}
