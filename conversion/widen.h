#ifndef CONVERSION_UTILITIES_WIDEN_H
#define CONVERSION_UTILITIES_WIDEN_H

#include "../global.h"

#include <functional>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

namespace ConversionUtilities {

/*!
 * \brief Converts a std::string to a wide string using the specified locale.
 * \deprecated Might be removed in future release because not used anymore. Use iconv based string converion functions instead.
 */
template <class E, class T = std::char_traits<E>, class A = std::allocator<E>>
class CPP_UTILITIES_EXPORT Widen : public std::unary_function<const std::string &, std::basic_string<E, T, A>> {
public:
    /*!
     * \brief Constructs a new instance with the specified \a locale.
     */
    Widen(const std::locale &locale = std::locale())
        : m_loc(locale)
        , m_pctype(&std::use_facet<std::ctype<E>>(locale))
    {
    }

    Widen(const Widen &) = delete;
    Widen &operator=(const Widen &) = delete;

    /*!
     * \brief Performs the conversation for the provided \a string.
     */
    std::basic_string<E, T, A> operator()(const std::string &string) const
    {
        typename std::basic_string<E, T, A>::size_type srcLen = string.length();
        const char *srcBeg = string.c_str();
        std::vector<E> tmp(srcLen);
        m_pctype->widen(srcBeg, srcBeg + srcLen, &tmp[0]);
        return std::basic_string<E, T, A>(&tmp[0], srcLen);
    }

private:
    std::locale m_loc;
    const std::ctype<E> *m_pctype;
};
} // namespace ConversionUtilities

#endif // CONVERSION_UTILITIES_WIDEN_H
