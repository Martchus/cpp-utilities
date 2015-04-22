#ifndef TIMESPAN_H
#define TIMESPAN_H

#include "../application/global.h"
#include "../conversion/types.h"

#include <string>

/*!
 * \brief Contains classes providing a means for handling date and time information.
 */
namespace ChronoUtilities
{

class DateTime;

/*!
 * \brief Specifies the output format.
 * \sa TimeSpan::toString()
 */
enum class TimeSpanOutputFormat
{
    Normal, /**< the normal form of specifing a time interval: hh:mm:ss */
    WithMeasures /**< measures are used, eg.: 34 d 5 h 10 min 7 s 31 ms */
};

class LIB_EXPORT TimeSpan
{
    friend class DateTime;
public:
    constexpr TimeSpan();
    constexpr TimeSpan(int64 ticks);

    static constexpr TimeSpan fromMilliseconds(double milliseconds);
    static constexpr TimeSpan fromSeconds(double seconds);
    static constexpr TimeSpan fromMinutes(double minutes);
    static constexpr TimeSpan fromHours(double hours);
    static constexpr TimeSpan fromDays(double days);
    static TimeSpan fromString(const std::string &str);
    static TimeSpan fromString(const std::string &str, char separator);

    constexpr int64 totalTicks() const;
    constexpr double totalMilliseconds() const;
    constexpr double totalSeconds() const;
    constexpr double totalMinutes() const;
    constexpr double totalHours() const;
    constexpr double totalDays() const;

    constexpr int milliseconds() const;
    constexpr int seconds() const;
    constexpr int minutes() const;
    constexpr int hours() const;
    constexpr int days() const;

    constexpr bool operator==(const TimeSpan &other) const;
    constexpr bool operator!=(const TimeSpan &other) const;
    constexpr bool operator<(const TimeSpan &other) const;
    constexpr bool operator>(const TimeSpan &other) const;
    constexpr bool operator<=(const TimeSpan &other) const;
    constexpr bool operator>=(const TimeSpan &other) const;
    constexpr TimeSpan operator+(const TimeSpan &other) const;
    constexpr TimeSpan operator-(const TimeSpan &other) const;
    TimeSpan &operator+=(const TimeSpan &other);
    TimeSpan &operator-=(const TimeSpan &other);

    std::string toString(TimeSpanOutputFormat format = TimeSpanOutputFormat::Normal, bool noMilliseconds = false) const;
    void toString(std::string &result, TimeSpanOutputFormat format = TimeSpanOutputFormat::Normal, bool noMilliseconds = false) const;
    constexpr bool isNull() const;
    constexpr bool isNegative() const;

private:
    int64 m_ticks;
    static constexpr uint64 m_ticksPerMillisecond = 10000uL;
    static constexpr uint64 m_ticksPerSecond = 10000000uL;
    static constexpr uint64 m_ticksPerMinute = 600000000uL;
    static constexpr uint64 m_ticksPerHour = 36000000000uL;
    static constexpr uint64 m_ticksPerDay = 864000000000uL;
};

/*!
 * Constructs a new instance of the TimeSpan class with zero ticks.
 */
constexpr inline TimeSpan::TimeSpan() : m_ticks(0)
{}

/*!
 * Constructs a new instance of the TimeSpan class with the specified number of ticks.
 */
constexpr inline TimeSpan::TimeSpan(int64 ticks) : m_ticks(ticks)
{}

constexpr inline TimeSpan TimeSpan::fromMilliseconds(double milliseconds)
{
    return TimeSpan(static_cast<int64>(milliseconds * static_cast<double>(m_ticksPerMillisecond)));
}

constexpr inline TimeSpan TimeSpan::fromSeconds(double seconds)
{
    return TimeSpan(static_cast<int64>(seconds * static_cast<double>(m_ticksPerSecond)));
}

constexpr inline TimeSpan TimeSpan::fromMinutes(double minutes)
{
    return TimeSpan(static_cast<int64>(minutes * static_cast<double>(m_ticksPerMinute)));
}

constexpr inline TimeSpan TimeSpan::fromHours(double hours)
{
    return TimeSpan(static_cast<int64>(hours * static_cast<double>(m_ticksPerHour)));
}

constexpr inline TimeSpan TimeSpan::fromDays(double days)
{
    return TimeSpan(static_cast<int64>(days * static_cast<double>(m_ticksPerDay)));
}

/*!
 * Gets the number of ticks that represent the value of the current TimeSpan class.
 */
constexpr inline int64 TimeSpan::totalTicks() const
{
    return m_ticks;
}

/*!
 * Gets the value of the current TimeSpan class expressed in whole and fractional milliseconds.
 */
constexpr inline double TimeSpan::totalMilliseconds() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerMillisecond);
}

/*!
 * Gets the value of the current TimeSpan class expressed in whole and fractional seconds.
 */
constexpr inline double TimeSpan::totalSeconds() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerSecond);
}

/*!
 * Gets the value of the current TimeSpan class expressed in whole and fractional minutes.
 */
constexpr inline double TimeSpan::totalMinutes() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerMinute);
}

/*!
 * Gets the value of the current TimeSpan class expressed in whole and fractional hours.
 */
constexpr inline double TimeSpan::totalHours() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerHour);
}

/*!
 * Gets the value of the current TimeSpan class expressed in whole and fractional days.
 */
constexpr inline double TimeSpan::totalDays() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerDay);
}

/*!
 * Gets the miliseconds component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::milliseconds() const
{
    return (m_ticks / m_ticksPerMillisecond) % 1000l;
}

/*!
 * Gets the seconds component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::seconds() const
{
    return (m_ticks / m_ticksPerSecond) % 60l;
}

/*!
 * Gets the minutes component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::minutes() const
{
    return (m_ticks / m_ticksPerMinute) % 60l;
}

/*!
 * Gets the hours component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::hours() const
{
    return (m_ticks / m_ticksPerHour) % 24l;
}

/*!
 * Gets the days component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::days() const
{
    return (m_ticks / m_ticksPerDay);
}

/*!
 * Indicates whether two TimeSpan instances are equal.
 */
constexpr inline bool TimeSpan::operator ==(const TimeSpan &other) const
{
    return m_ticks == other.m_ticks;
}

/*!
 * Indicates whether two TimeSpan instances are not equal.
 */
constexpr inline bool TimeSpan::operator !=(const TimeSpan &other) const
{
    return m_ticks != other.m_ticks;
}

/*!
 * Indicates whether a specified TimeSpan is less than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator <(const TimeSpan &other) const
{
    return m_ticks < other.m_ticks;
}

/*!
 * Indicates whether a specified TimeSpan is greater than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator >(const TimeSpan &other) const
{
    return m_ticks > other.m_ticks;
}

/*!
 * Indicates whether a specified TimeSpan is less or equal than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator <=(const TimeSpan &other) const
{
    return m_ticks <= other.m_ticks;
}

/*!
 * Indicates whether a specified TimeSpan is greater or equal than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator >=(const TimeSpan &other) const
{
    return m_ticks >= other.m_ticks;
}

/*!
 * Adds two TimeSpan instances.
 */
constexpr inline TimeSpan TimeSpan::operator +(const TimeSpan &other) const
{
    return TimeSpan(m_ticks + other.m_ticks);
}

/*!
 * Substracts two TimeSpan instances.
 */
constexpr inline TimeSpan TimeSpan::operator -(const TimeSpan &other) const
{
    return TimeSpan(m_ticks - other.m_ticks);
}

/*!
 * Adds another TimeSpan to the current instance.
 */
inline TimeSpan &TimeSpan::operator +=(const TimeSpan &other)
{
    m_ticks += other.m_ticks;
    return *this;
}

/*!
 * Substracts another TimeSpan from the current instance.
 */
inline TimeSpan &TimeSpan::operator -=(const TimeSpan &other)
{
    m_ticks -= other.m_ticks;
    return *this;
}

/*!
 * Returns ture if the time interval represented by the current TimeSpan class is null.
 */
constexpr inline bool TimeSpan::isNull() const
{
    return m_ticks == 0;
}

/*!
 * Returns ture if the time interval represented by the current TimeSpan class is negative.
 */
constexpr inline bool TimeSpan::isNegative() const
{
    return m_ticks < 0;
}

}

#endif // TIMESPAN_H
