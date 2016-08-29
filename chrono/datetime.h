#ifndef CHRONO_UTILITIES_DATETIME_H
#define CHRONO_UTILITIES_DATETIME_H

#include "./timespan.h"

#include "../conversion/types.h"

#include <string>
#include <limits>
#include <ctime>

namespace ChronoUtilities
{

/*!
 * \brief Specifies the output format.
 * \sa DateTime::toString()
 */
enum class DateTimeOutputFormat
{
    DateAndTime, /**< date and time */
    DateOnly, /**< date only */
    TimeOnly, /**< time only */
    DateTimeAndWeekday, /**< date with weekday and time */
    DateTimeAndShortWeekday /**< date with abbreviated weekday and time */
};

/*!
 * \brief Specifies the day of the week.
 * \sa DateTime::dayOfWeek()
 */
enum class DayOfWeek
{
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
 * \sa DateTime::getDatePart()
 */
enum class DatePart
{
    Year, /**< year */
    Month, /**< month */
    DayOfYear, /**< day of year */
    Day /**< day */
};

class CPP_UTILITIES_EXPORT DateTime
{
public:
    explicit constexpr DateTime();
    explicit constexpr DateTime(uint64 ticks);
    static DateTime fromDate(int year = 1, int month = 1, int day = 1);
    static DateTime fromTime(int hour = 0, int minute = 0, int second = 0, double millisecond = 0.0);
    static DateTime fromDateAndTime(int year = 1, int month = 1, int day = 1, int hour = 0, int minute = 0, int second = 0, double millisecond = 0.0);
    static DateTime fromString(const std::string &str);
    static DateTime fromTimeStamp(time_t timeStamp);
    static DateTime fromTimeStampGmt(time_t timeStamp);

    constexpr uint64 totalTicks() const;
    int year() const;
    int month() const;
    int day() const;
    int dayOfYear() const;
    constexpr DayOfWeek dayOfWeek() const;
    constexpr int hour() const;
    constexpr int minute() const;
    constexpr int second() const;
    constexpr int millisecond() const;
    constexpr bool isNull() const;
    constexpr TimeSpan timeOfDay() const;
    bool isLeapYear() const;
    constexpr bool isEternity() const;
    constexpr bool isSameDay(const DateTime &other) const;
    std::string toString(DateTimeOutputFormat format = DateTimeOutputFormat::DateAndTime, bool noMilliseconds = false) const;
    void toString(std::string &result, DateTimeOutputFormat format = DateTimeOutputFormat::DateAndTime, bool noMilliseconds = false) const;
    static const char *printDayOfWeek(DayOfWeek dayOfWeek, bool abbreviation = false);

    static constexpr DateTime eternity();
    static DateTime now();
    static DateTime gmtNow();
    constexpr static bool isLeapYear(int year);
    static int daysInMonth(int year, int month);

    constexpr bool operator ==(const DateTime &other) const;
    constexpr bool operator !=(const DateTime &other) const;
    constexpr bool operator <(const DateTime &other) const;
    constexpr bool operator >(const DateTime &other) const;
    constexpr bool operator <=(const DateTime &other) const;
    constexpr bool operator >=(const DateTime &other) const;
    constexpr DateTime operator +(const TimeSpan &timeSpan) const;
    constexpr DateTime operator -(const TimeSpan &timeSpan) const;
    constexpr TimeSpan operator +(const DateTime &other) const;
    constexpr TimeSpan operator -(const DateTime &other) const;
    DateTime &operator +=(const TimeSpan &timeSpan);
    DateTime &operator -=(const TimeSpan &timeSpan);

private:
    static uint64 dateToTicks(int year, int month, int day);
    static uint64 timeToTicks(int hour, int minute, int second, double millisecond);
    int getDatePart(DatePart part) const;

    uint64 m_ticks;
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
constexpr inline DateTime::DateTime() :
    m_ticks(0)
{}

/*!
 * \brief Constructs a DateTime with the specified number of \a ticks.
 */
constexpr inline DateTime::DateTime(uint64 ticks) :
    m_ticks(ticks)
{}

/*!
 * \brief Constructs a DateTime to the specified \a year, \a month, and \a day.
 */
inline DateTime DateTime::fromDate(int year, int month, int day)
{
    return DateTime(dateToTicks(year, month, day));
}

/*!
 * \brief Constructs a DateTime to the specified \a hour, \a minute, \a second and \a millisecond.
 */
inline DateTime DateTime::fromTime(int hour, int minute, int second, double millisecond)
{
    return DateTime(timeToTicks(hour, minute, second, millisecond));
}

/*!
 * \brief Constructs a DateTime to the specified \a year, \a month, \a day, \a hour, \a minute, \a second and \a millisecond.
 */
inline DateTime DateTime::fromDateAndTime(int year, int month, int day, int hour, int minute, int second, double millisecond)
{
    if(uint64 ticks = dateToTicks(year, month, day)) {
        return DateTime(ticks + timeToTicks(hour, minute, second, millisecond));
    }
    return DateTime();
}

/*!
 * \brief Gets the number of ticks which represent the value of the current instance.
 */
constexpr inline uint64 DateTime::totalTicks() const
{
    return m_ticks;
}

/*!
 * \brief Gets the year component of the date represented by this instance.
 */
inline int DateTime::year() const
{
    return getDatePart(DatePart::Year);
}

/*!
 * \brief Gets the month component of the date represented by this instance.
 */
inline int DateTime::month() const
{
    return getDatePart(DatePart::Month);
}

/*!
 * \brief Gets the day component of the date represented by this instance.
 */
inline int DateTime::day() const
{
    return getDatePart(DatePart::Day);
}

/*!
 * \brief Gets the day of the year represented by this instance.
 */
inline int DateTime::dayOfYear() const
{
    return getDatePart(DatePart::DayOfYear);
}

/*!
 * \brief Gets the day of the week represented by this instance.
 * \sa DayOfWeek
 */
constexpr inline DayOfWeek DateTime::dayOfWeek() const
{
    return static_cast<DayOfWeek>((m_ticks / TimeSpan::m_ticksPerDay) % 7l);
}

/*!
 * \brief Gets the hour component of the date represented by this instance.
 */
constexpr inline int DateTime::hour() const
{
    return m_ticks / TimeSpan::m_ticksPerHour % 24ul;
}

/*!
 *\brief  Gets the minute component of the date represented by this instance.
 */
constexpr inline int DateTime::minute() const
{
    return m_ticks / TimeSpan::m_ticksPerMinute % 60ul;
}

/*!
 * \brief Gets the second component of the date represented by this instance.
 */
constexpr inline int DateTime::second() const
{
    return m_ticks / TimeSpan::m_ticksPerSecond % 60ul;
}

/*!
 * \brief Gets the millisecond component of the date represented by this instance.
 */
constexpr inline int DateTime::millisecond() const
{
    return m_ticks / TimeSpan::m_ticksPerMillisecond % 1000ul;
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
 * \brief Gets the time of day as TimeSpan for this instance.
 */
constexpr inline TimeSpan DateTime::timeOfDay() const
{
    return TimeSpan(m_ticks % TimeSpan::m_ticksPerDay);
}

/*!
 * \brief Returns an indication whether the year of the dae represented by this instance is a leap year.
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
    return (year % 4 != 0)
            ? false
            : ((year % 100 == 0)
               ? (year % 400 == 0)
               : true);
}

/*!
 * \brief Returns the number of days in the specified \a month and \a year.
 */
inline int DateTime::daysInMonth(int year, int month)
{
    return (month >= 1 && month <= 12)
            ? (isLeapYear(year)
               ? m_daysInMonth366[month - 1]
               : m_daysInMonth365[month - 1])
            : (0);
}

/*!
 * \brief Returns and indication whether two DateTime instances represent the same day.
 */
constexpr inline bool DateTime::isSameDay(const DateTime &other) const
{
    return (m_ticks / TimeSpan::m_ticksPerDay) == (other.m_ticks / TimeSpan::m_ticksPerDay);
}

/*!
 * \brief Constructs a new instance of the DateTime class with the maximal number of ticks.
 */
constexpr inline DateTime DateTime::eternity()
{
    return DateTime(std::numeric_limits<decltype(m_ticks)>::max());
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the local time.
 */
inline DateTime DateTime::now()
{
    return DateTime::fromTimeStamp(time(nullptr));
}

/*!
 * \brief Returns a DateTime object that is set to the current date and time on this computer, expressed as the GMT time.
 */
inline DateTime DateTime::gmtNow()
{
    return DateTime::fromTimeStampGmt(time(nullptr));
}

/*!
 * \brief Indicates whether two DateTime instances are equal.
 */
constexpr inline bool DateTime::operator ==(const DateTime &other) const
{
    return m_ticks == other.m_ticks;
}

/*!
 * \brief Indicates whether two DateTime instances are not equal.
 */
constexpr inline bool DateTime::operator !=(const DateTime &other) const
{
    return m_ticks != other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is less than another specified DateTime.
 */
constexpr inline bool DateTime::operator <(const DateTime &other) const
{
    return m_ticks < other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is greater than another specified DateTime.
 */
constexpr inline bool DateTime::operator >(const DateTime &other) const
{
    return m_ticks > other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is less or equal than another specified DateTime.
 */
constexpr inline bool DateTime::operator <=(const DateTime &other) const
{
    return m_ticks <= other.m_ticks;
}

/*!
 * \brief Indicates whether a specified DateTime is greater or equal than another specified DateTime.
 */
constexpr inline bool DateTime::operator >=(const DateTime &other) const
{
    return m_ticks >= other.m_ticks;
}

/*!
 * \brief Adds another instance.
 * \returns The result is another DateTime.
 */
constexpr inline DateTime DateTime::operator +(const TimeSpan &timeSpan) const
{
    return DateTime(m_ticks + timeSpan.m_ticks);
}

/*!
 * \brief Substracts another instance.
 * \returns The result is another DateTime.
 */
constexpr inline DateTime DateTime::operator -(const TimeSpan &timeSpan) const
{
    return DateTime(m_ticks - timeSpan.m_ticks);
}

/*!
 * \brief Adds two instances.
 * \returns The result is a TimeSpan.
 */
constexpr inline TimeSpan DateTime::operator +(const DateTime &other) const
{
    return TimeSpan(m_ticks + other.m_ticks);
}

/*!
 * \brief Substracts two DateTime instances.
 * \returns The result is a TimeSpan.
 */
constexpr inline TimeSpan DateTime::operator -(const DateTime &other) const
{
    return TimeSpan(m_ticks - other.m_ticks);
}

/*!
 * \brief Adds a TimeSpan to the current instance.
 */
inline DateTime &DateTime::operator +=(const TimeSpan &timeSpan)
{
    m_ticks += timeSpan.m_ticks;
    return *this;
}

/*!
 * \brief Substracts a TimeSpan from the current instance.
 */
inline DateTime &DateTime::operator -=(const TimeSpan &timeSpan)
{
    m_ticks -= timeSpan.m_ticks;
    return *this;
}

}

#endif // CHRONO_UTILITIES_DATETIME_H
