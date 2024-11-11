#include "./datetime.h"

#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace CppUtilities {

const int DateTime::m_daysPerYear = 365;
const int DateTime::m_daysPer4Years = 1461;
const int DateTime::m_daysPer100Years = 36524;
const int DateTime::m_daysPer400Years = 146097;
const int DateTime::m_daysTo1601 = 584388;
const int DateTime::m_daysTo1899 = 693593;
const int DateTime::m_daysTo10000 = 3652059;
const int DateTime::m_daysToMonth365[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
const int DateTime::m_daysToMonth366[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
const int DateTime::m_daysInMonth365[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const int DateTime::m_daysInMonth366[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

template <typename num1, typename num2, typename num3> constexpr bool inRangeInclMax(num1 val, num2 min, num3 max)
{
    return (val) >= (min) && (val) <= (max);
}

template <typename num1, typename num2, typename num3> constexpr bool inRangeExclMax(num1 val, num2 min, num3 max)
{
    return (val) >= (min) && (val) < (max);
}

/*!
 * \class DateTime
 * \brief Represents an instant in time, typically expressed as a date and time of day.
 * \remarks
 *  - Time values are measured in 100-nanosecond units called ticks,
 *    and a particular date is the number of ticks since 12:00 midnight, January 1,
 *    0001 A.D. (C.E.) in the Gregorian Calendar (excluding ticks that would be added by leap seconds).
 *  - There is no time zone information associated. You need to keep track of the used time zone separately. That can
 *    be done by keeping an additional TimeSpan around which represents the delta to GMT or by simply using GMT everywhere
 *    in the program.
 *  - When constructing an instance via DateTime::fromTimeStamp(), DateTime::fromChronoTimePoint() or DateTime::fromIsoStringLocal()
 *    the time zone deltas are "baked into" the DateTime instance. For instance, the expression (DateTime::now() - DateTime::gmtNow())
 *    returns one hour in Germany during winter time (and *not* zero although both instances represent the current time).
 * \todo
 * - Allow to determine the date part for each component at once to prevent multiple
 *   invocations of getDatePart().
 */

/*!
 * \struct DateTimeExpression
 * \brief The DateTimeExpression struct holds information about a time expression (e.g. an ISO-8601 timestamp).
 *
 * This struct holds the date and time specification (DateTimeExpression::value), the time-zone delta
 * (DateTimeExpression::delta) and what parts are actually specified (DateTimeExpression::parts).
 *
 * The idea is to be able to preserve a time-zone delta and present parts when deserializing and then
 * serializing again. Note that it still does not allow to preserve the format of the original timestamp
 * exactly.
 */

/*!
 * \brief Constructs a new DateTime object with the local time from the specified UNIX \a timeStamp.
 */
DateTime DateTime::fromTimeStamp(time_t timeStamp)
{
    if (timeStamp) {
        struct tm *const timeinfo = localtime(&timeStamp);
        return DateTime::fromDateAndTime(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
            timeinfo->tm_sec < 60 ? timeinfo->tm_sec : 59, 0);
    } else {
        return DateTime();
    }
}

/*!
 * \brief Parses the given C-style string as DateTime.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 *
 * The expected format is something like "2012-02-29 15:34:20.033" or "2012/02/29 15:34:20.033". The
 * delimiters '-', ':' and '/' are exchangeable.
 *
 * \sa DateTime::fromIsoString()
 */
DateTime DateTime::fromString(const char *str)
{
    return DateTimeExpression::fromString(str).value;
}

/*!
 * \brief Parses the specified ISO date time denotation provided as C-style string.
 * \returns Returns a pair where the first value is the parsed date time and the second value
 *          the time zone designator (a time span which can be subtracted from the first value to get the UTC time).
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 * \remarks
 * - Parsing durations and time intervals is *not* supported.
 * - Truncated representations are *not* supported.
 * - Standardised extensions (ISO 8601-2:2019) are *not* supported.
 * \sa https://en.wikipedia.org/wiki/ISO_8601
 */
std::pair<DateTime, TimeSpan> DateTime::fromIsoString(const char *str)
{
    const auto expr = DateTimeExpression::fromIsoString(str);
    return std::make_pair(expr.value, expr.delta);
}

/*!
 * \brief Returns the string representation of the current instance using the specified \a format.
 * \remarks If \a noMilliseconds is true the date will be rounded to full seconds.
 * \sa toIsoString() for ISO format
 */
void DateTime::toString(string &result, DateTimeOutputFormat format, bool noMilliseconds) const
{
    if (format == DateTimeOutputFormat::Iso) {
        result = toIsoString();
        return;
    }

    stringstream s(stringstream::in | stringstream::out);
    s << setfill('0');

    if (format == DateTimeOutputFormat::IsoOmittingDefaultComponents) {
        constexpr auto dateDelimiter = '-', timeDelimiter = ':';
        const int components[] = { year(), month(), day(), hour(), minute(), second(), millisecond(), microsecond(), nanosecond() };
        const int *const firstTimeComponent = components + 3;
        const int *const firstFractionalComponent = components + 6;
        const int *const lastComponent = components + 8;
        const int *componentsEnd = noMilliseconds ? firstFractionalComponent : lastComponent + 1;
        for (const int *i = componentsEnd - 1; i > components; --i) {
            if (i >= firstTimeComponent && *i == 0) {
                componentsEnd = i;
            } else if (i < firstTimeComponent && *i == 1) {
                componentsEnd = i;
            }
        }
        for (const int *i = components; i != componentsEnd; ++i) {
            if (i == firstTimeComponent) {
                s << 'T';
            } else if (i == firstFractionalComponent) {
                s << '.';
            }
            if (i == components) {
                s << setw(4) << *i;
            } else if (i < firstFractionalComponent) {
                if (i < firstTimeComponent) {
                    s << dateDelimiter;
                } else if (i > firstTimeComponent) {
                    s << timeDelimiter;
                }
                s << setw(2) << *i;
            } else if (i < lastComponent) {
                s << setw(3) << *i;
            } else {
                s << *i / TimeSpan::nanosecondsPerTick;
            }
        }
        result = s.str();
        return;
    }

    if (format == DateTimeOutputFormat::DateTimeAndWeekday || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << printDayOfWeek(dayOfWeek(), format == DateTimeOutputFormat::DateTimeAndShortWeekday) << ' ';
    if (format == DateTimeOutputFormat::DateOnly || format == DateTimeOutputFormat::DateAndTime || format == DateTimeOutputFormat::DateTimeAndWeekday
        || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << setw(4) << year() << '-' << setw(2) << month() << '-' << setw(2) << day();
    if (format == DateTimeOutputFormat::DateAndTime || format == DateTimeOutputFormat::DateTimeAndWeekday
        || format == DateTimeOutputFormat::DateTimeAndShortWeekday)
        s << " ";
    if (format == DateTimeOutputFormat::TimeOnly || format == DateTimeOutputFormat::DateAndTime || format == DateTimeOutputFormat::DateTimeAndWeekday
        || format == DateTimeOutputFormat::DateTimeAndShortWeekday) {
        s << setw(2) << hour() << ':' << setw(2) << minute() << ':' << setw(2) << second();
        int ms = millisecond();
        if (!noMilliseconds && ms > 0) {
            s << '.' << setw(3) << ms;
        }
    }
    result = s.str();
}

/*!
 * \brief Returns the string representation of the current instance in the ISO format with custom delimiters,
 *        eg. 2016/08/29T21-32-31.588539814+02:00 with '/' as \a dateDelimiter and '-' as \a timeDelimiter.
 */
string DateTime::toIsoStringWithCustomDelimiters(TimeSpan timeZoneDelta, char dateDelimiter, char timeDelimiter, char timeZoneDelimiter) const
{
    stringstream s(stringstream::in | stringstream::out);
    s << setfill('0');
    s << setw(4) << year() << dateDelimiter << setw(2) << month() << dateDelimiter << setw(2) << day() << 'T' << setw(2) << hour() << timeDelimiter
      << setw(2) << minute() << timeDelimiter << setw(2) << second();
    const int milli(millisecond());
    const int micro(microsecond());
    const int nano(nanosecond());
    if (milli || micro || nano) {
        s << '.' << setw(3) << milli;
        if (micro || nano) {
            s << setw(3) << micro;
            if (nano) {
                s << nano / TimeSpan::nanosecondsPerTick;
            }
        }
    }
    if (!timeZoneDelta.isNull()) {
        if (timeZoneDelta.isNegative()) {
            s << '-';
            timeZoneDelta = TimeSpan(-timeZoneDelta.totalTicks());
        } else {
            s << '+';
        }
        s << setw(2) << timeZoneDelta.hours() << timeZoneDelimiter << setw(2) << timeZoneDelta.minutes();
    }
    return s.str();
}

/*!
 * \brief Returns the string representation of the current instance in the ISO format,
 *        eg. 2016-08-29T21:32:31.588539814+02:00.
 */
string DateTime::toIsoString(TimeSpan timeZoneDelta) const
{
    return toIsoStringWithCustomDelimiters(timeZoneDelta);
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
    if (abbreviation) {
        switch (dayOfWeek) {
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
        switch (dayOfWeek) {
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

#if defined(PLATFORM_UNIX) && !defined(PLATFORM_MAC)
/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the GMT time.
 * \remarks Only available under UNIX-like systems supporting clock_gettime().
 */
DateTime DateTime::exactGmtNow()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return DateTime(DateTime::unixEpochStart().totalTicks() + static_cast<std::uint64_t>(t.tv_sec) * TimeSpan::ticksPerSecond
        + static_cast<std::uint64_t>(t.tv_nsec) / 100);
}
#endif

/*!
 * \brief Converts the given date expressed in \a year, \a month and \a day to ticks.
 */
DateTime::TickType DateTime::dateToTicks(int year, int month, int day)
{
    if (!inRangeInclMax(year, 1, 9999)) {
        throw ConversionException("year is out of range");
    }
    if (!inRangeInclMax(month, 1, 12)) {
        throw ConversionException("month is out of range");
    }
    const auto *const daysToMonth = reinterpret_cast<const int *>(isLeapYear(year) ? m_daysToMonth366 : m_daysToMonth365);
    const int passedMonth = month - 1;
    if (!inRangeInclMax(day, 1, daysToMonth[month] - daysToMonth[passedMonth])) {
        throw ConversionException("day is out of range");
    }
    const auto passedYears = static_cast<unsigned int>(year - 1);
    const auto passedDays = static_cast<unsigned int>(day - 1);
    return (passedYears * m_daysPerYear + passedYears / 4 - passedYears / 100 + passedYears / 400
               + static_cast<unsigned int>(daysToMonth[passedMonth]) + passedDays)
        * TimeSpan::ticksPerDay;
}

/*!
 * \brief Converts the given time expressed in \a hour, \a minute, \a second and \a millisecond to ticks.
 */
DateTime::TickType DateTime::timeToTicks(int hour, int minute, int second, double millisecond)
{
    if (!inRangeExclMax(hour, 0, 24)) {
        throw ConversionException("hour is out of range");
    }
    if (!inRangeExclMax(minute, 0, 60)) {
        throw ConversionException("minute is out of range");
    }
    if (!inRangeExclMax(second, 0, 60)) {
        throw ConversionException("second is out of range");
    }
    if (!inRangeExclMax(millisecond, 0.0, 1000.0)) {
        throw ConversionException("millisecond is out of range");
    }
    return static_cast<std::uint64_t>(hour * TimeSpan::ticksPerHour) + static_cast<std::uint64_t>(minute * TimeSpan::ticksPerMinute)
        + static_cast<std::uint64_t>(second * TimeSpan::ticksPerSecond) + static_cast<std::uint64_t>(millisecond * TimeSpan::ticksPerMillisecond);
}

/*!
 * \brief Returns the specified date part.
 * \sa DatePart
 */
int DateTime::getDatePart(DatePart part) const
{
    const auto fullDays = static_cast<int>(m_ticks / TimeSpan::ticksPerDay);
    const auto full400YearBlocks = fullDays / m_daysPer400Years;
    const auto daysMinusFull400YearBlocks = fullDays - full400YearBlocks * m_daysPer400Years;
    auto full100YearBlocks = daysMinusFull400YearBlocks / m_daysPer100Years;
    if (full100YearBlocks == 4) {
        full100YearBlocks = 3;
    }
    const auto daysMinusFull100YearBlocks = daysMinusFull400YearBlocks - full100YearBlocks * m_daysPer100Years;
    const auto full4YearBlocks = daysMinusFull100YearBlocks / m_daysPer4Years;
    const auto daysMinusFull4YearBlocks = daysMinusFull100YearBlocks - full4YearBlocks * m_daysPer4Years;
    auto full1YearBlocks = daysMinusFull4YearBlocks / m_daysPerYear;
    if (full1YearBlocks == 4) {
        full1YearBlocks = 3;
    }
    if (part == DatePart::Year) {
        return full400YearBlocks * 400 + full100YearBlocks * 100 + full4YearBlocks * 4 + full1YearBlocks + 1;
    }
    const auto restDays = daysMinusFull4YearBlocks - full1YearBlocks * m_daysPerYear;
    if (part == DatePart::DayOfYear) { // day
        return restDays + 1;
    }
    const auto *const daysToMonth = (full1YearBlocks == 3 && (full4YearBlocks != 24 || full100YearBlocks == 3)) ? m_daysToMonth366 : m_daysToMonth365;
    auto month = 1;
    while (restDays >= daysToMonth[month]) {
        ++month;
    }
    if (part == DatePart::Month) {
        return month;
    } else if (part == DatePart::Day) {
        return restDays - daysToMonth[month - 1] + 1;
    }
    return 0;
}

/// \cond
static DateTimeParts dateTimePartsFromParsingDistance(const int *valueIndex, const int *values)
{
    return static_cast<DateTimeParts>((1 << (valueIndex - values + 1)) - 1);
}
/// \endcond

/*!
 * \brief Parses the specified ISO date time denotation provided as C-style string.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 * \remarks
 * - Parsing durations and time intervals is *not* supported.
 * - Truncated representations are *not* supported.
 * - Standardised extensions (ISO 8601-2:2019) are *not* supported.
 * \sa https://en.wikipedia.org/wiki/ISO_8601
 */
DateTimeExpression DateTimeExpression::fromIsoString(const char *str)
{
    auto res = DateTimeExpression();
    int values[9] = { 0 };
    int *const yearIndex = values + 0;
    int *const monthIndex = values + 1;
    int *const dayIndex = values + 2;
    int *const hourIndex = values + 3;
    int *const secondsIndex = values + 5;
    int *const miliSecondsIndex = values + 6;
    int *const deltaHourIndex = values + 7;
    int *const valuesEnd = values + 9;
    int *valueIndex = values;
    unsigned int remainingDigits = 4;
    bool deltaNegative = false;
    double millisecondsFact = 100.0, milliseconds = 0.0;
    for (const char *strIndex = str;; ++strIndex) {
        const char c = *strIndex;
        if (c <= '9' && c >= '0') {
            if (valueIndex == miliSecondsIndex) {
                milliseconds += (c - '0') * millisecondsFact;
                millisecondsFact /= 10;
            } else {
                if (!remainingDigits) {
                    if (++valueIndex == miliSecondsIndex || valueIndex >= valuesEnd) {
                        throw ConversionException("Max. number of digits exceeded");
                    }
                    remainingDigits = 2;
                }
                *valueIndex *= 10;
                *valueIndex += c - '0';
                remainingDigits -= 1;
            }
        } else if (c == 'T') {
            if (++valueIndex != hourIndex) {
                throw ConversionException("\"T\" expected before hour");
            }
            remainingDigits = 2;
        } else if (c == '-') {
            if (valueIndex < dayIndex) {
                ++valueIndex;
            } else if (++valueIndex >= secondsIndex) {
                valueIndex = deltaHourIndex;
                deltaNegative = true;
            } else {
                throw ConversionException("Unexpected \"-\" after day");
            }
            remainingDigits = 2;
        } else if (c == '.') {
            if (valueIndex != secondsIndex) {
                throw ConversionException("Unexpected \".\"");
            } else {
                ++valueIndex;
            }
        } else if (c == ':') {
            if (valueIndex < hourIndex) {
                throw ConversionException("Unexpected \":\" before hour");
            } else if (valueIndex == secondsIndex) {
                throw ConversionException("Unexpected \":\" after second");
            } else {
                ++valueIndex;
            }
            remainingDigits = 2;
        } else if ((c == '+') && (++valueIndex >= secondsIndex)) {
            valueIndex = deltaHourIndex;
            deltaNegative = false;
            remainingDigits = 2;
        } else if ((c == 'Z') && (++valueIndex >= secondsIndex)) {
            valueIndex = deltaHourIndex + 2;
            remainingDigits = 2;
        } else if (c == '\0') {
            break;
        } else {
            throw ConversionException(argsToString("Unexpected \"", c, '\"'));
        }
    }
    res.delta = TimeSpan::fromMinutes(*deltaHourIndex * 60 + values[8]);
    if (deltaNegative) {
        res.delta = TimeSpan(-res.delta.totalTicks());
    }
    if (valueIndex < monthIndex && !*monthIndex) {
        *monthIndex = 1;
    }
    if (valueIndex < dayIndex && !*dayIndex) {
        *dayIndex = 1;
    }
    res.value = DateTime::fromDateAndTime(*yearIndex, *monthIndex, *dayIndex, *hourIndex, values[4], *secondsIndex, milliseconds);
    res.parts = dateTimePartsFromParsingDistance(valueIndex, values);
    return res;
}

/*!
 * \brief Parses the given C-style string.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 *
 * The expected format is something like "2012-02-29 15:34:20.033" or "2012/02/29 15:34:20.033". The
 * delimiters '-', ':' and '/' are exchangeable.
 *
 * \sa DateTimeExpression::fromIsoString()
 */
DateTimeExpression DateTimeExpression::fromString(const char *str)
{
    auto res = DateTimeExpression();
    int values[7] = { 0 };
    int *const monthIndex = values + 1;
    int *const dayIndex = values + 2;
    int *const secondsIndex = values + 5;
    int *valueIndex = values;
    int *const valuesEnd = values + 7;
    double millisecondsFact = 100.0, milliseconds = 0.0;
    for (const char *strIndex = str;; ++strIndex) {
        const char c = *strIndex;
        if (c <= '9' && c >= '0') {
            if (valueIndex > secondsIndex) {
                milliseconds += (c - '0') * millisecondsFact;
                millisecondsFact /= 10;
            } else {
                Detail::raiseAndAdd(*valueIndex, 10, c);
            }
        } else if ((c == '-' || c == ':' || c == '/') || (c == '.' && (valueIndex == secondsIndex))
            || ((c == ' ' || c == 'T') && (valueIndex == dayIndex))) {
            if (++valueIndex == valuesEnd) {
                break; // just ignore further values for now
            }
        } else if (c == '\0') {
            break;
        } else {
            throw ConversionException(argsToString("Unexpected character \"", c, '\"'));
        }
    }
    if (valueIndex < monthIndex && !*monthIndex) {
        *monthIndex = 1;
    }
    if (valueIndex < dayIndex && !*dayIndex) {
        *dayIndex = 1;
    }
    res.value = DateTime::fromDateAndTime(values[0], values[1], *dayIndex, values[3], values[4], *secondsIndex, milliseconds);
    res.parts = dateTimePartsFromParsingDistance(valueIndex, values);
    return res;
}

/*!
 * \brief Returns the string representation of the current instance in the ISO format.
 * \remarks Only present parts will be included.
 */
std::string DateTimeExpression::toIsoString(char dateDelimiter, char timeDelimiter, char timeZoneDelimiter) const
{
    auto s = std::stringstream(std::stringstream::in | std::stringstream::out);
    s << setfill('0');
    if (parts && DateTimeParts::Year) {
        s << setw(4) << value.year();
    }
    if (parts && DateTimeParts::Month) {
        if (s.tellp()) {
            s << dateDelimiter;
        }
        s << setw(2) << value.month();
    }
    if (parts && DateTimeParts::Day) {
        if (s.tellp()) {
            s << dateDelimiter;
        }
        s << setw(2) << value.day();
    }
    if (parts && DateTimeParts::Hour) {
        if (s.tellp()) {
            s << 'T';
        }
        s << setw(2) << value.hour();
    }
    if (parts && DateTimeParts::Minute) {
        if (s.tellp()) {
            s << timeDelimiter;
        }
        s << setw(2) << value.minute();
    }
    if (parts && DateTimeParts::Second) {
        if (s.tellp()) {
            s << timeDelimiter;
        }
        s << setw(2) << value.second();
    }
    if (parts && DateTimeParts::SubSecond) {
        const auto milli = value.millisecond();
        const auto micro = value.microsecond();
        const auto nano = value.nanosecond();
        s << '.' << setw(3) << milli;
        if (micro || nano) {
            s << setw(3) << micro;
            if (nano) {
                s << nano / TimeSpan::nanosecondsPerTick;
            }
        }
    }
    if (parts && DateTimeParts::TimeZoneDelta) {
        auto d = delta;
        if (d.isNegative()) {
            s << '-';
            d = TimeSpan(-d.totalTicks());
        } else {
            s << '+';
        }
        if (parts && DateTimeParts::DeltaHour) {
            s << setw(2) << d.hours();
        }
        if (parts && DateTimeParts::DeltaMinute) {
            if (parts && DateTimeParts::DeltaHour) {
                s << timeZoneDelimiter;
            }
            s << setw(2) << d.minutes();
        }
    }
    return s.str();
}

} // namespace CppUtilities
