#ifndef MEMORY_H
#define MEMORY_H

#include <memory>

/// \cond

#if __cplusplus <= 201103L
#define __cpp_lib_make_unique 201304
namespace std {
template <typename _Tp> struct _MakeUniq {
    typedef unique_ptr<_Tp> __single_object;
};

template <typename _Tp> struct _MakeUniq<_Tp[]> {
    typedef unique_ptr<_Tp[]> __array;
};

template <typename _Tp, size_t _Bound> struct _MakeUniq<_Tp[_Bound]> {
    struct __invalid_type {
    };
};

/// std::make_unique for single objects
template <typename _Tp, typename... _Args> inline typename _MakeUniq<_Tp>::__single_object make_unique(_Args &&... __args)
{
    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

/// std::make_unique for arrays of unknown bound
template <typename _Tp> inline typename _MakeUniq<_Tp>::__array make_unique(size_t __num)
{
    return unique_ptr<_Tp>(new typename remove_extent<_Tp>::type[__num]());
}

/// Disable std::make_unique for arrays of known bound
template <typename _Tp, typename... _Args> inline typename _MakeUniq<_Tp>::__invalid_type make_unique(_Args &&...) = delete;
} // namespace std
#endif

/// \endcond

#endif // MEMORY_H
