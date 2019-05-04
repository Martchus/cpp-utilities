#ifndef CHRONO_UTILITIES_PERIOD_H
#define CHRONO_UTILITIES_PERIOD_H

#include "./datetime.h"

namespace ChronoUtilities {

class CPP_UTILITIES_EXPORT Period {
public:
    constexpr Period();
    Period(DateTime begin, DateTime end);
    constexpr int years() const;
    constexpr int months() const;
    constexpr int days() const;

private:
    int m_years;
    int m_months;
    int m_days;
};

constexpr Period::Period()
    : m_years(0)
    , m_months(0)
    , m_days(0)
{
}

/*!
 * \brief Returns the years component of the period represented by the current instance.
 */
constexpr int Period::years() const
{
    return m_years;
}

/*!
 * \brief Returns the months component of the period represented by the current instance.
 */
constexpr int Period::months() const
{
    return m_months;
}

/*!
 * \brief Returns the days component of the period represented by the current instance.
 */
constexpr int Period::days() const
{
    return m_days;
}

DateTime CPP_UTILITIES_EXPORT operator+(DateTime begin, Period period);

} // namespace ChronoUtilities

#endif // CHRONO_UTILITIES_PERIOD_H
