#ifndef CHRONO_FORMAT_H
#define CHRONO_FORMAT_H

#include "./datetime.h"

#include <ostream>

inline std::ostream &operator<<(std::ostream &out, const ChronoUtilities::DateTime &value)
{
    return out << value.toString(ChronoUtilities::DateTimeOutputFormat::DateAndTime, false);
}

inline std::ostream &operator<<(std::ostream &out, const ChronoUtilities::TimeSpan &value)
{
    return out << value.toString(ChronoUtilities::TimeSpanOutputFormat::Normal, false);
}

#endif // CHRONO_FORMAT_H
