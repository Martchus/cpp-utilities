#define CHRONO_UTILITIES_TIMESPAN_INTEGER_SCALE_OVERLOADS

#include "./timespan.h"

#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"

#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

namespace CppUtilities {

/*!
 * \class TimeSpan
 * \brief Represents a time interval.
 *
 * Note that the TimeSpan class is meant to express a time interval independently of the
 * concrete starting DateTime and end DateTime and hence can not be expressed in years
 * and month. For that use case, use the Period class instead.
 *
 * \remarks Time values are measured in 100-nanosecond units called ticks.
 * \todo
 * - Add method for parsing custom string formats.
 * - Add method for printing to custom string formats.
 */

/*!
 * \brief Parses the given C-style string as TimeSpan.
 * \throws Throws a ConversionException if the specified \a str does not match the expected format.
 *
 * The expected format is "days:hours:minutes:seconds", e.g. "5:31:4.521" for 5 hours, 31 minutes
 * and 4.521 seconds. So parts at the front can be omitted and the parts can be fractions. The
 * colon can be changed by specifying another \a separator. White-spaces before and after parts
 * are ignored.
 *
 * It is also possible to specify one or more values with a unit, e.g. "2w 1d 5h 1m 0.5s".
 * The units "w" (weeks), "d" (days), "h" (hours), "m" (minutes) and "s" (seconds) are supported.
 */
TimeSpan TimeSpan::fromString(const char *str, char separator)
{
    if (!*str) {
        return TimeSpan();
    }

    auto parts = std::array<double, 4>();
    auto partsPresent = std::size_t();
    auto specificationsWithUnits = TimeSpan();

    for (const char *i = str;; ++i) {
        // skip over white-spaces
        if (*i == ' ' && i == str) {
            str = i + 1;
            continue;
        }

        // consider non-separator and non-terminator characters as part to be interpreted as number
        if (*i != separator && *i != '\0') {
            continue;
        }

        // allow only up to 4 parts (days, hours, minutes and seconds)
        if (partsPresent == 4) {
            throw ConversionException("too many separators/parts");
        }

        // parse value of the part
        auto part = 0.0;
        auto valueWithUnit = TimeSpan();
        if (str != i) {
            // parse value of the part as double
            const auto res = std::from_chars(str, i, part);
            if (res.ec != std::errc()) {
                const auto part = std::string_view(str, i - str);
                if (res.ec == std::errc::result_out_of_range) {
                    throw ConversionException(argsToString("part \"", part, "\" is too large"));
                } else {
                    throw ConversionException(argsToString("part \"", part, "\" cannot be interpreted as floating point number"));
                }
            }
            // handle remaining characters; detect a possibly present unit suffix
            for (const char *suffix = res.ptr; suffix != i; ++suffix) {
                if (*suffix == ' ') {
                    continue;
                }
                if (valueWithUnit.isNull()) {
                    switch (*suffix) {
                    case 'w':
                        valueWithUnit = TimeSpan::fromDays(7.0 * part);
                        continue;
                    case 'd':
                        valueWithUnit = TimeSpan::fromDays(part);
                        continue;
                    case 'h':
                        valueWithUnit = TimeSpan::fromHours(part);
                        continue;
                    case 'm':
                        valueWithUnit = TimeSpan::fromMinutes(part);
                        continue;
                    case 's':
                        valueWithUnit = TimeSpan::fromSeconds(part);
                        continue;
                    default:
                        ;
                    }
                }
                if (*suffix >= '0' && *suffix <= '9') {
                    str = i = suffix;
                    break;
                }
                throw ConversionException(argsToString("unexpected character \"", *suffix, '\"'));
            }
        }

        // set part value; add value with unit
        if (valueWithUnit.isNull()) {
            parts[partsPresent++] = part;
        } else {
            specificationsWithUnits += valueWithUnit;
        }

        // expect next part starting after the separator or stop if terminator reached
        if (*i == separator) {
            str = i + 1;
        } else if (*i == '\0') {
            break;
        }
    }

    // compute and return total value from specifications with units and parts
    switch (partsPresent) {
    case 1:
        return specificationsWithUnits + TimeSpan::fromSeconds(parts.front());
    case 2:
        return specificationsWithUnits + TimeSpan::fromMinutes(parts.front()) + TimeSpan::fromSeconds(parts[1]);
    case 3:
        return specificationsWithUnits + TimeSpan::fromHours(parts.front()) + TimeSpan::fromMinutes(parts[1]) + TimeSpan::fromSeconds(parts[2]);
    default:
        return specificationsWithUnits + TimeSpan::fromDays(parts.front()) + TimeSpan::fromHours(parts[1]) + TimeSpan::fromMinutes(parts[2]) + TimeSpan::fromSeconds(parts[3]);
    }
}

/*!
 * \brief Converts the value of the current TimeSpan object to its equivalent std::string representation
 *        according the given \a format.
 *
 * If \a fullSeconds is true the time interval will be rounded to full seconds.
 *
 * The string representation will be stored in \a result.
 */
void TimeSpan::toString(string &result, TimeSpanOutputFormat format, bool fullSeconds) const
{
    stringstream s(stringstream::in | stringstream::out);
    TimeSpan positive(m_ticks);
    if (positive.isNegative()) {
        s << '-';
        positive.m_ticks = -positive.m_ticks;
    }
    switch (format) {
    case TimeSpanOutputFormat::Normal:
        s << setfill('0') << setw(2) << floor(positive.totalHours()) << ":" << setw(2) << positive.minutes() << ":" << setw(2) << positive.seconds();
        if (!fullSeconds) {
            const int milli(positive.milliseconds());
            const int micro(positive.microseconds());
            const int nano(positive.nanoseconds());
            if (milli || micro || nano) {
                s << '.' << setw(3) << milli;
                if (micro || nano) {
                    s << setw(3) << micro;
                    if (nano) {
                        s << nano / TimeSpan::nanosecondsPerTick;
                    }
                }
            }
        }
        break;
    case TimeSpanOutputFormat::WithMeasures:
        if (isNull()) {
            result = "0 s";
            return;
        } else {
            if (!fullSeconds && positive.totalMilliseconds() < 1.0) {
                s << setprecision(2) << positive.totalMicroseconds() << " µs";
            } else {
                bool needWhitespace = false;
                if (const int days = positive.days()) {
                    needWhitespace = true;
                    s << days << " d";
                }
                if (const int hours = positive.hours()) {
                    if (needWhitespace)
                        s << ' ';
                    needWhitespace = true;
                    s << hours << " h";
                }
                if (const int minutes = positive.minutes()) {
                    if (needWhitespace)
                        s << ' ';
                    needWhitespace = true;
                    s << minutes << " min";
                }
                if (const int seconds = positive.seconds()) {
                    if (needWhitespace)
                        s << ' ';
                    needWhitespace = true;
                    s << seconds << " s";
                }
                if (!fullSeconds) {
                    if (const int milliseconds = positive.milliseconds()) {
                        if (needWhitespace)
                            s << ' ';
                        needWhitespace = true;
                        s << milliseconds << " ms";
                    }
                    if (const int microseconds = positive.microseconds()) {
                        if (needWhitespace)
                            s << ' ';
                        needWhitespace = true;
                        s << microseconds << " µs";
                    }
                    if (const int nanoseconds = positive.nanoseconds()) {
                        if (needWhitespace)
                            s << ' ';
                        s << nanoseconds << " ns";
                    }
                }
            }
        }
        break;
    case TimeSpanOutputFormat::TotalSeconds:
        if (fullSeconds) {
            s << setprecision(0);
        } else {
            s << setprecision(10);
        }
        s << positive.totalSeconds();
        break;
    }
    result = s.str();
}

} // namespace CppUtilities
