#ifndef CHRONO_FORMAT_H
#define CHRONO_FORMAT_H

#include "./datetime.h"
#include "./period.h"

#include <ostream>

inline std::ostream &operator<<(std::ostream &out, const ChronoUtilities::DateTime &value)
{
    return out << value.toString(ChronoUtilities::DateTimeOutputFormat::DateAndTime, false);
}

inline std::ostream &operator<<(std::ostream &out, const ChronoUtilities::TimeSpan &value)
{
    return out << value.toString(ChronoUtilities::TimeSpanOutputFormat::Normal, false);
}

inline std::ostream &operator<<(std::ostream &out, const ChronoUtilities::Period &value)
{
    return out << value.years() << " years, " << value.months() << " months, " << value.days() << " days";
}

#endif // CHRONO_FORMAT_H
