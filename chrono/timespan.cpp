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
 * \remarks Time values are measured in 100-nanosecond units called ticks.
 */

/*!
 * \brief Parses the given C-style string as TimeSpan.
 */
TimeSpan TimeSpan::fromString(const char *str, char separator)
{
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
    case 0:
        return TimeSpan();
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
 * If \a noMilliseconds is true the time interval will be rounded to full seconds.
 */
string TimeSpan::toString(TimeSpanOutputFormat format, bool noMilliseconds) const
{
    string result;
    toString(result, format, noMilliseconds);
    return result;
}

/*!
 * \brief Converts the value of the current TimeSpan object to its equivalent std::string representation
 *        according the given \a format.
 *
 * If \a noMilliseconds is true the time interval will be rounded to full seconds.
 *
 * The string representation will be stored in \a result.
 */
void TimeSpan::toString(string &result, TimeSpanOutputFormat format, bool noMilliseconds) const
{
    stringstream s(stringstream::in | stringstream::out);
    if (isNegative())
        s << "- ";
    switch (format) {
    case TimeSpanOutputFormat::Normal:
        s << setfill('0') << setw(2) << floor(fabs(totalHours())) << ":" << setw(2) << minutes() << ":" << setw(2) << seconds() << " ";
        break;
    case TimeSpanOutputFormat::WithMeasures:
        if (isNull()) {
            s << "0 s ";
        } else if (totalMilliseconds() < 1.0) {
            s << setprecision(2) << (m_ticks / 10.0) << " µs ";
        } else {
            if (days()) {
                s << days() << " d ";
            }
            if (hours()) {
                s << hours() << " h ";
            }
            if (minutes()) {
                s << minutes() << " min ";
            }
            if (seconds()) {
                s << seconds() << " s ";
            }
            if (!noMilliseconds && milliseconds()) {
                s << milliseconds() << " ms ";
            }
        }
        break;
    }
    result = s.str().substr(0, static_cast<string::size_type>(s.tellp()) - 1);
}
