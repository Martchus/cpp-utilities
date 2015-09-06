#include "./timespan.h"

#include "../conversion/stringconversion.h"

#include <sstream>
#include <cmath>
#include <iomanip>
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
 * Parses the given std::string \a str as TimeSpan.
 */
TimeSpan TimeSpan::fromString(const string &str)
{
    return TimeSpan::fromString(str, ':');
}

/*!
 * Parses the given std::string \a str as TimeSpan.
 */
TimeSpan TimeSpan::fromString(const string &str, char separator)
{
    vector<double> parts;
    string::size_type start = 0;
    string::size_type end = str.find(separator, start);
    while(true) {
        parts.push_back(stringToNumber<double>(str.substr(start, end)));
        if(end == string::npos)
            break;
        start = end + 1;
        if(start >= str.size())
            break;
        end = str.find(separator, start);
    }
    switch(parts.size()) {
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
 * Converts the value of the current TimeSpan object to its equivalent std::string representation
 * according the given \a format.
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
 * Converts the value of the current TimeSpan object to its equivalent std::string representation
 * according the given \a format.
 *
 * If \a noMilliseconds is true the time interval will be rounded to full seconds.
 *
 * The string representation will be stored in \a result.
 */
void TimeSpan::toString(string &result, TimeSpanOutputFormat format, bool noMilliseconds) const
{
    stringstream s(stringstream::in | stringstream::out);
    if(isNegative())
        s << "- ";
    switch(format) {
    case TimeSpanOutputFormat::Normal:
        s << setfill('0') << setw(2) << floor(fabs(totalHours())) << ":" << setw(2) << minutes() << ":" << setw(2) << seconds() << " ";
        break;
    case TimeSpanOutputFormat::WithMeasures:
        if(isNull()) {
            s << "0 s ";
        } else if(totalMilliseconds() < 1.0) {
            s << setprecision(2) << (m_ticks / 10.0) << " µs ";
        } else {
            if(days()) {
                s << days() << " d ";
            }
            if(hours()) {
                s << hours() << " h ";
            }
            if(minutes()) {
                s << minutes() << " min ";
            }
            if(seconds()) {
                s << seconds() << " s ";
            }
            if(!noMilliseconds && milliseconds()) {
                s << milliseconds() << " ms ";
            }
        }
        break;
    }
    result = s.str().substr(0, static_cast<string::size_type>(s.tellp()) - 1);
}


