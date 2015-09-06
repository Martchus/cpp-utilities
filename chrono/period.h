#ifndef PERIOD_H
#define PERIOD_H

#include "./datetime.h"

#include "../application/global.h"

namespace ChronoUtilities {

class LIB_EXPORT Period
{
public:
    Period(const DateTime &beg, const DateTime &end);
    int years() const;
    int months() const;
    int days() const;
private:
    int m_years;
    int m_months;
    int m_days;
};

/*!
 * Gets the years component of the period represented by the current instance.
 */
inline int Period::years() const
{
    return m_years;
}

/*!
 * Gets the months component of the period represented by the current instance.
 */
inline int Period::months() const
{
    return m_months;
}

/*!
 * Gets the days component of the period represented by the current instance.
 */
inline int Period::days() const
{
    return m_days;
}

}

#endif // PERIOD_H
