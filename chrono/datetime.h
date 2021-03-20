#ifndef CHRONO_UTILITIES_DATETIME_H
#define CHRONO_UTILITIES_DATETIME_H

#include "./timespan.h"

#include <cstdint>
#include <ctime>
#include <limits>
#include <string>

namespace CppUtilities {

/*!
 * \brief Specifies the output format.
 * \sa DateTime::toString()
 */
enum class DateTimeOutputFormat {
    DateAndTime, /**< date and time */
    DateOnly, /**< date only */
    TimeOnly, /**< time only */
    DateTimeAndWeekday, /**< date with weekday and time */
    DateTimeAndShortWeekday, /**< date with abbreviated weekday and time */
    Iso, /**< ISO format like DateTime::toIsoString() */
    IsoOmittingDefaultComponents, /**< ISO format like DateTime::toIsoString() omitting default components, e.g. just "2017" instead of "2017-01-01T00:00:00" */
};

/*!
 * \brief Specifies the day of the week.
 * \sa DateTime::dayOfWeek()
 */
enum class DayOfWeek {
    Monday, /**< Monday */
    Tuesday, /**< Tuesday */
    Wednesday, /**< Wednesday */
    Thursday, /**< Thursday */
    Friday, /**< Friday */
    Saturday, /**< Saturday */
    Sunday /**< Sunday */
};

/*!
 * \brief Specifies the date part.
 * \remarks Intended for internal use only.
 * \sa DateTime::getDatePart()
 */
enum class DatePart {
    Year, /**< year */
    Month, /**< month */
    DayOfYear, /**< day of year */
    Day /**< day */
};

class CPP_UTILITIES_EXPORT DateTime {
public:
    explicit constexpr DateTime();
    explicit constexpr DateTime(std::uint64_t ticks);
    static DateTime fromDate(int year = 1, int month = 1, int day = 1);
    static DateTime fromTime(int hour = 0, int minute = 0, int second = 0, double millisecond = 0.0);
    static DateTime fromDateAndTime(int year = 1, int month = 1, int day = 1, int hour = 0, int minute = 0, int second = 0, double millisecond = 0.0);
    static DateTime fromString(const std::string &str);
    static DateTime fromString(const char *str);
    static std::pair<DateTime, TimeSpan> fromIsoString(const char *str);
    static DateTime fromIsoStringGmt(const char *str);
    static DateTime fromIsoStringLocal(const char *str);
    static DateTime fromTimeStamp(std::time_t timeStamp);
    constexpr static DateTime fromTimeStampGmt(std::time_t timeStamp);
    template <typename TimePoint> static DateTime fromChronoTimePoint(TimePoint timePoint);
    template <typename TimePoint> constexpr static DateTime fromChronoTimePointGmt(TimePoint timePoint);

    constexpr std::uint64_t &ticks();
    constexpr std::uint64_t totalTicks() const;
    int year() const;
    int month() const;
    int day() const;
    int dayOfYear() const;
    constexpr DayOfWeek dayOfWeek() const;
    constexpr int hour() const;
    constexpr int minute() const;
    constexpr int second() const;
    constexpr int millisecond() const;
    constexpr int microsecond() const;
    constexpr int nanosecond() const;
    constexpr bool isNull() const;
    constexpr TimeSpan timeOfDay() const;
    bool isLeapYear() const;
    constexpr bool isEternity() const;
    constexpr bool isSameDay(const DateTime &other) const;
    std::string toString(DateTimeOutputFormat format = DateTimeOutputFormat::DateAndTime, bool noMilliseconds = false) const;
    void toString(std::string &result, DateTimeOutputFormat format = DateTimeOutputFormat::DateAndTime, bool noMilliseconds = false) const;
    std::string toIsoStringWithCustomDelimiters(
        TimeSpan timeZoneDelta = TimeSpan(), char dateDelimiter = '-', char timeDelimiter = ':', char timeZoneDelimiter = ':') const;
    std::string toIsoString(TimeSpan timeZoneDelta = TimeSpan()) const;
    constexpr std::time_t toTimeStamp() const;
    static const char *printDayOfWeek(DayOfWeek dayOfWeek, bool abbreviation = false);

    static constexpr DateTime eternity();
    static constexpr DateTime unixEpochStart();
    static DateTime now();
    static DateTime gmtNow();
#if defined(PLATFORM_UNIX) && !defined(PLATFORM_MAC)
    static DateTime exactGmtNow();
#endif
    constexpr static bool isLeapYear(int year);
    static int daysInMonth(int year, int month);

    constexpr bool operator==(const DateTime &other) const;
    constexpr bool operator!=(const DateTime &other) const;
    constexpr bool operator<(const DateTime &other) const;
    constexpr bool operator>(const DateTime &other) const;
    constexpr bool operator<=(const DateTime &other) const;
    constexpr bool operator>=(const DateTime &other) const;
    constexpr DateTime operator+(const TimeSpan &timeSpan) const;
    constexpr DateTime operator-(const TimeSpan &timeSpan) const;
    constexpr TimeSpan operator+(const DateTime &other) const;
    constexpr TimeSpan operator-(const DateTime &other) const;
    DateTime &operator+=(const TimeSpan &timeSpan);
    DateTime &operator-=(const TimeSpan &timeSpan);

private:
    static std::uint64_t dateToTicks(int year, int month, int day);
    static std::uint64_t timeToTicks(int hour, int minute, int second, double millisecond);
    int getDatePart(DatePart part) const;

    std::uint64_t m_ticks;
    static const int m_daysPerYear;
    static const int m_daysPer4Years;
    static const int m_daysPer100Years;
    static const int m_daysPer400Years;
    static const int m_daysTo1601;
    static const int m_daysTo1899;
    static const int m_daysTo10000;
    static const int m_daysToMonth365[13];
    static const int m_daysToMonth366[13];
    static const int m_daysInMonth365[12];
    static const int m_daysInMonth366[12];
};

/*!
 * \brief Constructs a DateTime.
 */
constexpr inline DateTime::DateTime()
    : m_ticks(0)
{
}

/*!
 * \brief Constructs a DateTime with the specified number of \a ticks.
 */
constexpr inline DateTime::DateTime(std::uint64_t ticks)
    : m_ticks(ticks)
{
}

/*!
 * \brief Constructs a DateTime to the specified \a year, \a month, and \a day.
 * \throws Throws a ConversionException if the specified \a year, \a month or \a day is out-of-range.
 */
inline DateTime DateTime::fromDate(int year, int month, int day)
{
    return DateTime(dateToTicks(year, month, day));
}

/*!
 * \brief Constructs a DateTime to the specified \a hour, \a minute, \a second and \a millisecond.
 * \throws Throws a ConversionException if the specified \a hour, \a minute, \a second or \a millisecond is out-of-range.
 */
inline DateTime DateTime::fromTime(int hour, int minute, int second, double millisecond)
{
    return DateTime(timeToTicks(hour, minute, second, millisecond));
}

/*!
 * \brief Constructs a DateTime to the specified \a year, \a month, \a day, \a hour, \a minute, \a second and \a millisecond.
 * \throws Throws a ConversionException if the specified \a year, \a month, \a day, \a hour, \a minute, \a second or \a millisecond
 *         is out-of-range.
 */
inline DateTime DateTime::fromDateAndTime(int year, int month, int day, int hour, int minute, int second, double millisecond)
{
    return DateTime(dateToTicks(year, month, day) + timeToTicks(hour, minute, second, millisecond));
}

/*!
 * \brief Parses the given std::string as DateTime.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 *
 * The expected format is something like "2012-02-29 15:34:20.033" or "2012/02/29 15:34:20.033". The
 * delimiters '-', ':' and '/' are exchangeable.
 *
 * \sa DateTime::fromIsoString()
 */
inline DateTime DateTime::fromString(const std::string &str)
{
    return fromString(str.data());
}

/*!
 * \brief Parses the specified ISO date time denotation provided as C-style string.
 * \returns Returns the parsed UTC time. That means a possibly denoted time zone delta is subtracted from the time stamp.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 * \sa fromIsoString()
 */
inline DateTime DateTime::fromIsoStringGmt(const char *str)
{
    const auto tmp = fromIsoString(str);
    return tmp.first - tmp.second;
}

/*!
 * \brief Parses the specified ISO date time denotation provided as C-style string.
 * \returns Returns the parsed local time. That means a possibly denoted time zone delta is discarded.
 * \throws Throws a ConversionException if the specified \a str does not match the expected time format.
 * \sa fromIsoString()
 */
inline DateTime DateTime::fromIsoStringLocal(const char *str)
{
    return fromIsoString(str).first;
}

/*!
 * \brief Constructs a new DateTime object with the GMT time from the specified UNIX \a timeStamp.
 */
constexpr inline DateTime DateTime::fromTimeStampGmt(std::time_t timeStamp)
{
    return DateTime(DateTime::unixEpochStart().totalTicks() + static_cast<std::uint64_t>(timeStamp) * TimeSpan::ticksPerSecond);
}

/*!
 * \brief Constructs a new DateTime object with the local time from the specified std::chrono::time_point.
 * \remarks Works only with time points of std::chrono::system_clock so far. C++20 will fix this. Until then this function
 *          should be considered experimental.
 */
template <typename TimePoint> inline DateTime DateTime::fromChronoTimePoint(TimePoint timePoint)
{
    return DateTime::fromTimeStamp(decltype(timePoint)::clock::to_time_t(timePoint));
}

/*!
 * \brief Constructs a new DateTime object with the GMT time from the specified std::chrono::time_point.
 * \remarks Works only with time points of std::chrono::system_clock so far. C++20 will fix this. Until then this function
 *          should be considered experimental.
 */
template <typename TimePoint> constexpr DateTime DateTime::fromChronoTimePointGmt(TimePoint timePoint)
{
    return DateTime::fromTimeStampGmt(decltype(timePoint)::clock::to_time_t(timePoint));
}

/*!
 * \brief Returns a mutable reference to the total ticks.
 */
constexpr inline std::uint64_t &DateTime::ticks()
{
    return m_ticks;
}

/*!
 * \brief Returns the number of ticks which represent the value of the current instance.
 */
constexpr inline std::uint64_t DateTime::totalTicks() const
{
    return m_ticks;
}

/*!
 * \brief Returns the year component of the date represented by this instance.
 */
inline int DateTime::year() const
{
    return getDatePart(DatePart::Year);
}

/*!
 * \brief Returns the month component of the date represented by this instance.
 */
inline int DateTime::month() const
{
    return getDatePart(DatePart::Month);
}

/*!
 * \brief Returns the day component of the date represented by this instance.
 */
inline int DateTime::day() const
{
    return getDatePart(DatePart::Day);
}

/*!
 * \brief Returns the day of the year represented by this instance.
 */
inline int DateTime::dayOfYear() const
{
    return getDatePart(DatePart::DayOfYear);
}

/*!
 * \brief Returns the day of the week represented by this instance.
 * \sa DayOfWeek
 */
constexpr inline DayOfWeek DateTime::dayOfWeek() const
{
    return static_cast<DayOfWeek>((m_ticks / TimeSpan::ticksPerDay) % 7l);
}

/*!
 * \brief Returns the hour component of the date represented by this instance.
 */
constexpr inline int DateTime::hour() const
{
    return static_cast<int>(m_ticks / TimeSpan::ticksPerHour % 24ul);
}

/*!
 *\brief Returns the minute component of the date represented by this instance.
 */
constexpr inline int DateTime::minute() const
{
    return static_cast<int>(m_ticks / TimeSpan::ticksPerMinute % 60ul);
}

/*!
 * \brief Returns the second component of the date represented by this instance.
 */
constexpr inline int DateTime::second() const
{
    return static_cast<int>(m_ticks / TimeSpan::ticksPerSecond % 60ul);
}

/*!
 * \brief Returns the millisecond component of the date represented by this instance.
 */
constexpr inline int DateTime::millisecond() const
{
    return static_cast<int>(m_ticks / TimeSpan::ticksPerMillisecond % 1000ul);
}

/*!
 * \brief Returns the microsecond component of the date represented by this instance.
 */
constexpr inline int DateTime::microsecond() const
{
    return static_cast<int>(m_ticks / TimeSpan::ticksPerMicrosecond % 1000ul);
}

/*!
 * \brief Returns the nanosecond component of the date represented by this instance.
 * \remarks The accuracy of the DateTime class is 100-nanoseconds. Hence the returned value
 *          will always have two zeros at the end (in decimal representation).
 */
constexpr inline int DateTime::nanosecond() const
{
    return static_cast<int>(m_ticks % 10ul * TimeSpan::nanosecondsPerTick);
}

/*!
 * \brief Returns ture if the date represented by the current DateTime class is null.
 * \sa DateTime
 */
constexpr inline bool DateTime::isNull() const
{
    return m_ticks == 0;
}

/*!
 * \brief Returns the time of day as TimeSpan for this instance.
 */
constexpr inline TimeSpan DateTime::timeOfDay() const
{
    return TimeSpan(static_cast<std::int64_t>(m_ticks % TimeSpan::ticksPerDay));
}

/*!
 * \brief Returns an indication whether the year represented by this instance is a leap year.
 */
inline bool DateTime::isLeapYear() const
{
    return isLeapYear(year());
}

/*!
 * \brief Returns whether the instance has the maximal number of ticks.
 */
constexpr inline bool DateTime::isEternity() const
{
    return m_ticks == std::numeric_limits<decltype(m_ticks)>::max();
}

/*!
 * \brief Returns an indication whether the specified \a year is a leap year.
 */
constexpr inline bool DateTime::isLeapYear(int year)
{
    return (year % 4 != 0) ? false : ((year % 100 == 0) ? (year % 400 == 0) : true);
}

/*!
 * \brief Returns the number of days in the specified \a month and \a year.
 */
inline int DateTime::daysInMonth(int year, int month)
{
    return (month >= 1 && month <= 12) ? (isLeapYear(year) ? m_daysInMonth366[month - 1] : m_daysInMonth365[month - 1]) : (0);
}

/*!
 * \brief Returns and indication whether two DateTime instances represent the same day.
 */
constexpr inline bool DateTime::isSameDay(const DateTime &other) const
{
    return (m_ticks / TimeSpan::ticksPerDay) == (other.m_ticks / TimeSpan::ticksPerDay);
}

/*!
 * \brief Returns the string representation of the current instance using the specified \a format.
 * \remarks If \a noMilliseconds is true the date will be rounded to full seconds.
 * \sa toIsoString() for ISO format
 */
inline std::string DateTime::toString(DateTimeOutputFormat format, bool noMilliseconds) const
{
    std::string result;
    toString(result, format, noMilliseconds);
    return result;
}

/*!
 * \brief Returns the UNIX timestamp for the current instance.
 */
constexpr std::time_t DateTime::toTimeStamp() const
{
    return static_cast<std::time_t>((totalTicks() - DateTime::unixEpochStart().totalTicks()) / TimeSpan::ticksPerSecond);
}

/*!
 * \brief Constructs a new instance of the DateTime class with the maximal number of ticks.
 */
constexpr inline DateTime DateTime::eternity()
{
    return DateTime(std::numeric_limits<decltype(m_ticks)>::max());
}

/*!
 * \brief Returns the DateTime object for the "1970-01-01T00:00:00Z".
 */
constexpr inline DateTime DateTime::unixEpochStart()
{
    return DateTime(621355968000000000);
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the local time.
 * \remarks The time might be rounded to full seconds. Use exactGmtNow() for better precision.
 */
inline DateTime DateTime::now()
{
    return DateTime::fromTimeStamp(std::time(nullptr));
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the GMT time.
 * \remarks The time might be rounded to full seconds. Use exactGmtNow() for better precision.
 */
inline DateTime DateTime::gmtNow()
{
    return DateTime::fromTimeStampGmt(std::time(nullptr));
}

/*!
 * \brief Indicates whether two DateTime instances are equal.
 */
constexpr inline bool DateTime::operator==(const DateTime &other) const
{
    return m_ticks == other.m_ticks;
}

/*!
 * \brief Indicates whether two DateTime instances are not equal.
 */
constexpr inline bool DateTime::operator!=(const DateTime &other) const
{
    return m_ticks != other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is less than another specified DateTime.
 */
constexpr inline bool DateTime::operator<(const DateTime &other) const
{
    return m_ticks < other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is greater than another specified DateTime.
 */
constexpr inline bool DateTime::operator>(const DateTime &other) const
{
    return m_ticks > other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is less or equal than another specified DateTime.
 */
constexpr inline bool DateTime::operator<=(const DateTime &other) const
{
    return m_ticks <= other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is greater or equal than another specified DateTime.
 */
constexpr inline bool DateTime::operator>=(const DateTime &other) const
{
    return m_ticks >= other.m_ticks;
}

/*!
 * \brief Adds another instance.
 * \returns The result is another DateTime.
 */
constexpr inline DateTime DateTime::operator+(const TimeSpan &timeSpan) const
{
    return DateTime(m_ticks + static_cast<std::uint64_t>(timeSpan.m_ticks));
}

/*!
 * \brief Substracts another instance.
 * \returns The result is another DateTime.
 */
constexpr inline DateTime DateTime::operator-(const TimeSpan &timeSpan) const
{
    return DateTime(m_ticks - static_cast<std::uint64_t>(timeSpan.m_ticks));
}

/*!
 * \brief Adds two instances.
 * \returns The result is a TimeSpan.
 */
constexpr inline TimeSpan DateTime::operator+(const DateTime &other) const
{
    return TimeSpan(static_cast<std::int64_t>(m_ticks + other.m_ticks));
}

/*!
 * \brief Substracts two DateTime instances.
 * \returns The result is a TimeSpan.
 * \remarks For expressing the delta between two concrete DateTime instances in terms of
 *          years, month and days, use Period::Period instead.
 */
constexpr inline TimeSpan DateTime::operator-(const DateTime &other) const
{
    return TimeSpan(static_cast<std::int64_t>(m_ticks - other.m_ticks));
}

/*!
 * \brief Adds a TimeSpan to the current instance.
 */
inline DateTime &DateTime::operator+=(const TimeSpan &timeSpan)
{
    m_ticks += static_cast<std::uint64_t>(timeSpan.m_ticks);
    return *this;
}

/*!
 * \brief Substracts a TimeSpan from the current instance.
 */
inline DateTime &DateTime::operator-=(const TimeSpan &timeSpan)
{
    m_ticks += static_cast<std::uint64_t>(timeSpan.m_ticks);
    return *this;
}
} // namespace CppUtilities

namespace std {
/// \brief Computes the hash for the CppUtilities::DateTime instance.
template <> struct hash<CppUtilities::DateTime> {
    inline size_t operator()(const CppUtilities::DateTime &dateTime) const
    {
        return hash<decltype(dateTime.totalTicks())>()(dateTime.totalTicks());
    }
};
} // namespace std

#endif // CHRONO_UTILITIES_DATETIME_H
