#include "./timespan.h"

#include "../conversion/stringconversion.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;
using namespace ChronoUtilities;
using namespace ConversionUtilities;

/*!
 * \class ChronoUtilities::TimeSpan
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
 * The expected format is "days:hours:minutes:seconds", eg. "5:31:4.521" for 5 hours, 31 minutes
 * and 4.521 seconds. So parts at the front can be omitted and the parts can be fractions. The
 * colon can be changed by specifying another \a separator.
 */
TimeSpan TimeSpan::fromString(const char *str, char separator)
{
    if (!*str) {
        return TimeSpan();
    }

    vector<double> parts;
    size_t partsSize = 1;
    for (const char *i = str; *i; ++i) {
        *i == separator && ++partsSize;
    }
    parts.reserve(partsSize);

    for (const char *i = str;;) {
        if (*i == separator) {
            parts.emplace_back(stringToNumber<double>(string(str, i)));
            str = ++i;
        } else if (*i == '\0') {
            parts.emplace_back(stringToNumber<double>(string(str, i)));
            break;
        } else {
            ++i;
        }
    }

    switch (parts.size()) {
    case 1:
        return TimeSpan::fromSeconds(parts.front());
    case 2:
        return TimeSpan::fromMinutes(parts.front()) + TimeSpan::fromSeconds(parts[1]);
    case 3:
        return TimeSpan::fromHours(parts.front()) + TimeSpan::fromMinutes(parts[1]) + TimeSpan::fromSeconds(parts[2]);
    default:
        return TimeSpan::fromDays(parts.front()) + TimeSpan::fromHours(parts[1]) + TimeSpan::fromMinutes(parts[2]) + TimeSpan::fromSeconds(parts[3]);
    }
}

/*!
 * \brief Converts the value of the current TimeSpan object to its equivalent std::string representation
 *        according the given \a format.
 *
 * If \a fullSeconds is true the time interval will be rounded to full seconds.
 */
string TimeSpan::toString(TimeSpanOutputFormat format, bool fullSeconds) const
{
    string result;
    toString(result, format, fullSeconds);
    return result;
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
