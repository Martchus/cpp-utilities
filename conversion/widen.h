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
 * Converts a std::string to std::wstring.
 */
template<class E, class T = std::char_traits<E>, class A = std::allocator<E> >
class LIB_EXPORT Widen : public std::unary_function<
        const std::string&, std::basic_string<E, T, A> >
{
public:
    /*!
     * Constructs a new instance.
     */
    Widen(const std::locale& loc = std::locale()) :
        m_loc(loc)
    {
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6.0...
        using namespace std;
        m_pctype = &_USE(loc, ctype<E> );
#else
        m_pctype = &std::use_facet<std::ctype<E> >(loc);
#endif
    }

    Widen(const Widen &) = delete;
    Widen& operator= (const Widen &) = delete;

    /*!
     * Performs the conversation for the provided \a string.
     */
    std::basic_string<E, T, A> operator() (const std::string& string) const
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
