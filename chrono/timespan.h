#ifndef CHRONO_UTILITIES_TIMESPAN_H
#define CHRONO_UTILITIES_TIMESPAN_H

#include "../conversion/types.h"
#include "../global.h"

#include <functional>
#include <limits>
#include <string>

/*!
 * \brief Contains classes providing a means for handling date and time information.
 */
namespace ChronoUtilities {

class DateTime;

/*!
 * \brief Specifies the output format.
 * \sa TimeSpan::toString()
 */
enum class TimeSpanOutputFormat {
    Normal, /**< the normal form of specifing a time interval: hh:mm:ss */
    WithMeasures, /**< measures are used, eg.: 34 d 5 h 10 min 7 s 31 ms */
    TotalSeconds, /**< total seconds (as returned by totalSeconds()), eg. 2304.342 */
};

class CPP_UTILITIES_EXPORT TimeSpan {
    friend class DateTime;

public:
    explicit constexpr TimeSpan();
    explicit constexpr TimeSpan(int64 ticks);

    static constexpr TimeSpan fromMilliseconds(double milliseconds);
    static constexpr TimeSpan fromSeconds(double seconds);
    static constexpr TimeSpan fromMinutes(double minutes);
    static constexpr TimeSpan fromHours(double hours);
    static constexpr TimeSpan fromDays(double days);
    static TimeSpan fromString(const std::string &str, char separator = ':');
    static TimeSpan fromString(const char *str, char separator);
    static constexpr TimeSpan negativeInfinity();
    static constexpr TimeSpan infinity();

    int64 &ticks();
    constexpr int64 totalTicks() const;
    constexpr double totalMicroseconds() const;
    constexpr double totalMilliseconds() const;
    constexpr double totalSeconds() const;
    constexpr double totalMinutes() const;
    constexpr double totalHours() const;
    constexpr double totalDays() const;

    constexpr int nanoseconds() const;
    constexpr int microseconds() const;
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

    std::string toString(TimeSpanOutputFormat format = TimeSpanOutputFormat::Normal, bool fullSeconds = false) const;
    void toString(std::string &result, TimeSpanOutputFormat format = TimeSpanOutputFormat::Normal, bool fullSeconds = false) const;
    constexpr bool isNull() const;
    constexpr bool isNegative() const;
    constexpr bool isNegativeInfinity() const;
    constexpr bool isInfinity() const;

    // TODO: make those public constants signed in next major release and remove private ones then
    static constexpr int64 nanosecondsPerTick = 100uL;
    static constexpr int64 ticksPerMicrosecond = 10uL;
    static constexpr uint64 ticksPerMillisecond = 10000uL;
    static constexpr uint64 ticksPerSecond = 10000000uL;
    static constexpr uint64 ticksPerMinute = 600000000uL;
    static constexpr uint64 ticksPerHour = 36000000000uL;
    static constexpr uint64 ticksPerDay = 864000000000uL;

private:
    static constexpr int64 m_ticksPerMillisecond = 10000L;
    static constexpr int64 m_ticksPerSecond = 10000000L;
    static constexpr int64 m_ticksPerMinute = 600000000L;
    static constexpr int64 m_ticksPerHour = 36000000000L;
    static constexpr int64 m_ticksPerDay = 864000000000L;

private:
    int64 m_ticks;
};

/*!
 * \brief Constructs a new instance of the TimeSpan class with zero ticks.
 */
constexpr inline TimeSpan::TimeSpan()
    : m_ticks(0)
{
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of ticks.
 */
constexpr inline TimeSpan::TimeSpan(int64 ticks)
    : m_ticks(ticks)
{
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of miliseconds.
 */
constexpr inline TimeSpan TimeSpan::fromMilliseconds(double milliseconds)
{
    return TimeSpan(static_cast<int64>(milliseconds * static_cast<double>(m_ticksPerMillisecond)));
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of seconds.
 */
constexpr inline TimeSpan TimeSpan::fromSeconds(double seconds)
{
    return TimeSpan(static_cast<int64>(seconds * static_cast<double>(m_ticksPerSecond)));
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of minutes.
 */
constexpr inline TimeSpan TimeSpan::fromMinutes(double minutes)
{
    return TimeSpan(static_cast<int64>(minutes * static_cast<double>(m_ticksPerMinute)));
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of hours.
 */
constexpr inline TimeSpan TimeSpan::fromHours(double hours)
{
    return TimeSpan(static_cast<int64>(hours * static_cast<double>(m_ticksPerHour)));
}

/*!
 * \brief Constructs a new instance of the TimeSpan class with the specified number of days.
 */
constexpr inline TimeSpan TimeSpan::fromDays(double days)
{
    return TimeSpan(static_cast<int64>(days * static_cast<double>(m_ticksPerDay)));
}

/*!
 * \brief Parses the given std::string as TimeSpan.
 * \throws Throws a ConversionException if the specified \a str does not match the expected format.
 *
 * The expected format is "days:hours:minutes:seconds", eg. "5:31:4.521" for 5 hours, 31 minutes
 * and 4.521 seconds. So parts at the front can be omitted and the parts can be fractions. The
 * colon can be changed by specifying another \a separator.
 */
inline TimeSpan TimeSpan::fromString(const std::string &str, char separator)
{
    return TimeSpan::fromString(str.data(), separator);
}

/*!
 * \brief Constructs a new instace of the TimeSpan class with the minimal number of ticks.
 */
constexpr inline TimeSpan TimeSpan::negativeInfinity()
{
    return TimeSpan(std::numeric_limits<decltype(m_ticks)>::min());
}

/*!
 * \brief Constructs a new instace of the TimeSpan class with the maximal number of ticks.
 */
constexpr inline TimeSpan TimeSpan::infinity()
{
    return TimeSpan(std::numeric_limits<decltype(m_ticks)>::max());
}

/*!
 * \brief Returns a mutable reference to the total ticks.
 */
inline int64 &TimeSpan::ticks()
{
    return m_ticks;
}

/*!
 * \brief Returns the number of ticks that represent the value of the current TimeSpan class.
 */
constexpr inline int64 TimeSpan::totalTicks() const
{
    return m_ticks;
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional microseconds.
 */
constexpr double TimeSpan::totalMicroseconds() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(ticksPerMicrosecond);
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional milliseconds.
 */
constexpr inline double TimeSpan::totalMilliseconds() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerMillisecond);
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional seconds.
 */
constexpr inline double TimeSpan::totalSeconds() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerSecond);
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional minutes.
 */
constexpr inline double TimeSpan::totalMinutes() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerMinute);
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional hours.
 */
constexpr inline double TimeSpan::totalHours() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerHour);
}

/*!
 * \brief Returns the value of the current TimeSpan class expressed in whole and fractional days.
 */
constexpr inline double TimeSpan::totalDays() const
{
    return static_cast<double>(m_ticks) / static_cast<double>(m_ticksPerDay);
}

/*!
 * \brief Returns the nanoseconds component of the time interval represented by the current TimeSpan class.
 * \remarks The accuracy of the TimeSpan class is 100-nanoseconds. Hence the returned value
 *          will always have two zeros at the end (in decimal representation).
 */
constexpr int TimeSpan::nanoseconds() const
{
    return m_ticks % 10l * TimeSpan::nanosecondsPerTick;
}

/*!
 * \brief Returns the microseconds component of the time interval represented by the current TimeSpan class.
 */
constexpr int TimeSpan::microseconds() const
{
    return (m_ticks / ticksPerMicrosecond) % 1000l;
}

/*!
 * \brief Returns the miliseconds component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::milliseconds() const
{
    return (m_ticks / m_ticksPerMillisecond) % 1000l;
}

/*!
 * \brief Returns the seconds component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::seconds() const
{
    return (m_ticks / m_ticksPerSecond) % 60l;
}

/*!
 * \brief Returns the minutes component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::minutes() const
{
    return (m_ticks / m_ticksPerMinute) % 60l;
}

/*!
 * \brief Returns the hours component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::hours() const
{
    return (m_ticks / m_ticksPerHour) % 24l;
}

/*!
 * \brief Returns the days component of the time interval represented by the current TimeSpan class.
 */
constexpr inline int TimeSpan::days() const
{
    return (m_ticks / m_ticksPerDay);
}

/*!
 * \brief Indicates whether two TimeSpan instances are equal.
 */
constexpr inline bool TimeSpan::operator==(const TimeSpan &other) const
{
    return m_ticks == other.m_ticks;
}

/*!
 * \brief Indicates whether two TimeSpan instances are not equal.
 */
constexpr inline bool TimeSpan::operator!=(const TimeSpan &other) const
{
    return m_ticks != other.m_ticks;
}

/*!
 * \brief Indicates whether a specified TimeSpan is less than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator<(const TimeSpan &other) const
{
    return m_ticks < other.m_ticks;
}

/*!
 * \brief Indicates whether a specified TimeSpan is greater than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator>(const TimeSpan &other) const
{
    return m_ticks > other.m_ticks;
}

/*!
 * \brief Indicates whether a specified TimeSpan is less or equal than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator<=(const TimeSpan &other) const
{
    return m_ticks <= other.m_ticks;
}

/*!
 * \brief Indicates whether a specified TimeSpan is greater or equal than another specified TimeSpan.
 */
constexpr inline bool TimeSpan::operator>=(const TimeSpan &other) const
{
    return m_ticks >= other.m_ticks;
}

/*!
 * \brief Adds two TimeSpan instances.
 */
constexpr inline TimeSpan TimeSpan::operator+(const TimeSpan &other) const
{
    return TimeSpan(m_ticks + other.m_ticks);
}

/*!
 * \brief Substracts two TimeSpan instances.
 */
constexpr inline TimeSpan TimeSpan::operator-(const TimeSpan &other) const
{
    return TimeSpan(m_ticks - other.m_ticks);
}

/*!
 * \brief Adds another TimeSpan to the current instance.
 */
inline TimeSpan &TimeSpan::operator+=(const TimeSpan &other)
{
    m_ticks += other.m_ticks;
    return *this;
}

/*!
 * \brief Substracts another TimeSpan from the current instance.
 */
inline TimeSpan &TimeSpan::operator-=(const TimeSpan &other)
{
    m_ticks -= other.m_ticks;
    return *this;
}

/*!
 * \brief Returns ture if the time interval represented by the current TimeSpan class is null.
 */
constexpr inline bool TimeSpan::isNull() const
{
    return m_ticks == 0;
}

/*!
 * \brief Returns ture if the time interval represented by the current TimeSpan class is negative.
 */
constexpr inline bool TimeSpan::isNegative() const
{
    return m_ticks < 0;
}

/*!
 * \brief Returns whether the time inverval represented by the current instance is the smallest representable TimeSpan.
 */
constexpr inline bool TimeSpan::isNegativeInfinity() const
{
    return m_ticks == std::numeric_limits<decltype(m_ticks)>::min();
}

/*!
 * \brief Returns whether the time inverval represented by the current instance is the longest representable TimeSpan.
 */
constexpr inline bool TimeSpan::isInfinity() const
{
    return m_ticks == std::numeric_limits<decltype(m_ticks)>::max();
}
} // namespace ChronoUtilities

namespace std {
template <> struct hash<ChronoUtilities::TimeSpan> {
    inline size_t operator()(const ChronoUtilities::TimeSpan &timeSpan) const
    {
        return hash<decltype(timeSpan.totalTicks())>()(timeSpan.totalTicks());
    }
};
} // namespace std

#endif // CHRONO_UTILITIES_TIMESPAN_H
