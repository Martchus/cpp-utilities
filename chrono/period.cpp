#include "./period.h"

namespace ChronoUtilities {

/*!
 * \class ChronoUtilities::Period
 * \brief Represents a period of time.
 */

/*!
 * \brief Constructs a new Period defined by a start DateTime and an end DateTime.
 */
Period::Period(const DateTime &beg, const DateTime &end)
{
    m_years = end.year() - beg.year();
    m_months = end.month() - beg.month();
    m_days = end.day() - beg.day();
    if (end.hour() < beg.hour()) {
        --m_days;
    }
    if (m_days < 0) {
        m_days += DateTime::daysInMonth(beg.year(), beg.month());
        --m_months;
    }
    if (m_months < 0) {
        m_months += 12;
        --m_years;
    }
}
}
