#ifndef WIDEN_H
#define WIDEN_H

#include "../application/global.h"

#include <string>
#include <vector>
#include <locale>
#include <functional>
#include <iostream>

namespace ConversionUtilities
{

/*!
 * \brief Converts a std::string to a wide string using the specified locale.
 */
template<class E, class T = std::char_traits<E>, class A = std::allocator<E> >
class LIB_EXPORT Widen : public std::unary_function<const std::string &, std::basic_string<E, T, A> >
{
public:
    /*!
     * \brief Constructs a new instance with the specified \a locale.
     */
    Widen(const std::locale &locale = std::locale()) :
        m_loc(locale),
        m_pctype(&std::use_facet<std::ctype<E> >(locale))
    {}

    Widen(const Widen &) = delete;
    Widen& operator= (const Widen &) = delete;

    /*!
     * \brief Performs the conversation for the provided \a string.
     */
    std::basic_string<E, T, A> operator() (const std::string &string) const
    {
        typename std::basic_string<E, T, A>::size_type srcLen = string.length();
        const char *srcBeg = string.c_str();
        std::vector<E> tmp(srcLen);
        m_pctype->widen(srcBeg, srcBeg + srcLen, &tmp[0]);
        return std::basic_string<E, T, A>(&tmp[0], srcLen);
    }

private:
    std::locale m_loc;
    const std::ctype<E>* m_pctype;
};

}

#endif // WIDEN_H
