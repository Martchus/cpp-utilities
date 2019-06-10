#ifndef CHRONO_FORMAT_H
#define CHRONO_FORMAT_H

#include "./datetime.h"
#include "./period.h"

#include <ostream>

inline std::ostream &operator<<(std::ostream &out, const CppUtilities::DateTime &value)
{
    return out << value.toString(CppUtilities::DateTimeOutputFormat::DateAndTime, false);
}

inline std::ostream &operator<<(std::ostream &out, const CppUtilities::TimeSpan &value)
{
    return out << value.toString(CppUtilities::TimeSpanOutputFormat::Normal, false);
}

inline std::ostream &operator<<(std::ostream &out, const CppUtilities::Period &value)
{
    return out << value.years() << " years, " << value.months() << " months, " << value.days() << " days";
}

#endif // CHRONO_FORMAT_H
