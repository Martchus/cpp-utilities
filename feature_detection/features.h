// This file has originally been generated using the WriteCompilerDetectionHeader
// CMake module. It is now provided directly because the module has been deprecated.

#ifndef CPP_UTILITIES_COMPILER_DETECTION_H
#define CPP_UTILITIES_COMPILER_DETECTION_H

#ifdef __cplusplus
# define CPP_UTILITIES_COMPILER_IS_Comeau 0
# define CPP_UTILITIES_COMPILER_IS_Intel 0
# define CPP_UTILITIES_COMPILER_IS_IntelLLVM 0
# define CPP_UTILITIES_COMPILER_IS_PathScale 0
# define CPP_UTILITIES_COMPILER_IS_Embarcadero 0
# define CPP_UTILITIES_COMPILER_IS_Borland 0
# define CPP_UTILITIES_COMPILER_IS_Watcom 0
# define CPP_UTILITIES_COMPILER_IS_OpenWatcom 0
# define CPP_UTILITIES_COMPILER_IS_SunPro 0
# define CPP_UTILITIES_COMPILER_IS_HP 0
# define CPP_UTILITIES_COMPILER_IS_Compaq 0
# define CPP_UTILITIES_COMPILER_IS_zOS 0
# define CPP_UTILITIES_COMPILER_IS_XLClang 0
# define CPP_UTILITIES_COMPILER_IS_XL 0
# define CPP_UTILITIES_COMPILER_IS_VisualAge 0
# define CPP_UTILITIES_COMPILER_IS_NVHPC 0
# define CPP_UTILITIES_COMPILER_IS_PGI 0
# define CPP_UTILITIES_COMPILER_IS_Cray 0
# define CPP_UTILITIES_COMPILER_IS_TI 0
# define CPP_UTILITIES_COMPILER_IS_Fujitsu 0
# define CPP_UTILITIES_COMPILER_IS_GHS 0
# define CPP_UTILITIES_COMPILER_IS_SCO 0
# define CPP_UTILITIES_COMPILER_IS_ARMCC 0
# define CPP_UTILITIES_COMPILER_IS_AppleClang 0
# define CPP_UTILITIES_COMPILER_IS_ARMClang 0
# define CPP_UTILITIES_COMPILER_IS_Clang 0
# define CPP_UTILITIES_COMPILER_IS_GNU 0
# define CPP_UTILITIES_COMPILER_IS_MSVC 0
# define CPP_UTILITIES_COMPILER_IS_ADSP 0
# define CPP_UTILITIES_COMPILER_IS_IAR 0
# define CPP_UTILITIES_COMPILER_IS_MIPSpro 0

#if defined(__COMO__)
# undef CPP_UTILITIES_COMPILER_IS_Comeau
# define CPP_UTILITIES_COMPILER_IS_Comeau 1

#elif defined(__INTEL_COMPILER) || defined(__ICC)
# undef CPP_UTILITIES_COMPILER_IS_Intel
# define CPP_UTILITIES_COMPILER_IS_Intel 1

#elif (defined(__clang__) && defined(__INTEL_CLANG_COMPILER)) || defined(__INTEL_LLVM_COMPILER)
# undef CPP_UTILITIES_COMPILER_IS_IntelLLVM
# define CPP_UTILITIES_COMPILER_IS_IntelLLVM 1

#elif defined(__PATHCC__)
# undef CPP_UTILITIES_COMPILER_IS_PathScale
# define CPP_UTILITIES_COMPILER_IS_PathScale 1

#elif defined(__BORLANDC__) && defined(__CODEGEARC_VERSION__)
# undef CPP_UTILITIES_COMPILER_IS_Embarcadero
# define CPP_UTILITIES_COMPILER_IS_Embarcadero 1

#elif defined(__BORLANDC__)
# undef CPP_UTILITIES_COMPILER_IS_Borland
# define CPP_UTILITIES_COMPILER_IS_Borland 1

#elif defined(__WATCOMC__) && __WATCOMC__ < 1200
# undef CPP_UTILITIES_COMPILER_IS_Watcom
# define CPP_UTILITIES_COMPILER_IS_Watcom 1

#elif defined(__WATCOMC__)
# undef CPP_UTILITIES_COMPILER_IS_OpenWatcom
# define CPP_UTILITIES_COMPILER_IS_OpenWatcom 1

#elif defined(__SUNPRO_CC)
# undef CPP_UTILITIES_COMPILER_IS_SunPro
# define CPP_UTILITIES_COMPILER_IS_SunPro 1

#elif defined(__HP_aCC)
# undef CPP_UTILITIES_COMPILER_IS_HP
# define CPP_UTILITIES_COMPILER_IS_HP 1

#elif defined(__DECCXX)
# undef CPP_UTILITIES_COMPILER_IS_Compaq
# define CPP_UTILITIES_COMPILER_IS_Compaq 1

#elif defined(__IBMCPP__) && defined(__COMPILER_VER__)
# undef CPP_UTILITIES_COMPILER_IS_zOS
# define CPP_UTILITIES_COMPILER_IS_zOS 1

#elif defined(__ibmxl__) && defined(__clang__)
# undef CPP_UTILITIES_COMPILER_IS_XLClang
# define CPP_UTILITIES_COMPILER_IS_XLClang 1

#elif defined(__IBMCPP__) && !defined(__COMPILER_VER__) && __IBMCPP__ >= 800
# undef CPP_UTILITIES_COMPILER_IS_XL
# define CPP_UTILITIES_COMPILER_IS_XL 1

#elif defined(__IBMCPP__) && !defined(__COMPILER_VER__) && __IBMCPP__ < 800
# undef CPP_UTILITIES_COMPILER_IS_VisualAge
# define CPP_UTILITIES_COMPILER_IS_VisualAge 1

#elif defined(__NVCOMPILER)
# undef CPP_UTILITIES_COMPILER_IS_NVHPC
# define CPP_UTILITIES_COMPILER_IS_NVHPC 1

#elif defined(__PGI)
# undef CPP_UTILITIES_COMPILER_IS_PGI
# define CPP_UTILITIES_COMPILER_IS_PGI 1

#elif defined(_CRAYC)
# undef CPP_UTILITIES_COMPILER_IS_Cray
# define CPP_UTILITIES_COMPILER_IS_Cray 1

#elif defined(__TI_COMPILER_VERSION__)
# undef CPP_UTILITIES_COMPILER_IS_TI
# define CPP_UTILITIES_COMPILER_IS_TI 1

#elif defined(__FUJITSU) || defined(__FCC_VERSION) || defined(__fcc_version)
# undef CPP_UTILITIES_COMPILER_IS_Fujitsu
# define CPP_UTILITIES_COMPILER_IS_Fujitsu 1

#elif defined(__ghs__)
# undef CPP_UTILITIES_COMPILER_IS_GHS
# define CPP_UTILITIES_COMPILER_IS_GHS 1

#elif defined(__SCO_VERSION__)
# undef CPP_UTILITIES_COMPILER_IS_SCO
# define CPP_UTILITIES_COMPILER_IS_SCO 1

#elif defined(__ARMCC_VERSION) && !defined(__clang__)
# undef CPP_UTILITIES_COMPILER_IS_ARMCC
# define CPP_UTILITIES_COMPILER_IS_ARMCC 1

#elif defined(__clang__) && defined(__apple_build_version__)
# undef CPP_UTILITIES_COMPILER_IS_AppleClang
# define CPP_UTILITIES_COMPILER_IS_AppleClang 1

#elif defined(__clang__) && defined(__ARMCOMPILER_VERSION)
# undef CPP_UTILITIES_COMPILER_IS_ARMClang
# define CPP_UTILITIES_COMPILER_IS_ARMClang 1

#elif defined(__clang__)
# undef CPP_UTILITIES_COMPILER_IS_Clang
# define CPP_UTILITIES_COMPILER_IS_Clang 1

#elif defined(__GNUC__) || defined(__GNUG__)
# undef CPP_UTILITIES_COMPILER_IS_GNU
# define CPP_UTILITIES_COMPILER_IS_GNU 1

#elif defined(_MSC_VER)
# undef CPP_UTILITIES_COMPILER_IS_MSVC
# define CPP_UTILITIES_COMPILER_IS_MSVC 1

#elif defined(__VISUALDSPVERSION__) || defined(__ADSPBLACKFIN__) || defined(__ADSPTS__) || defined(__ADSP21000__)
# undef CPP_UTILITIES_COMPILER_IS_ADSP
# define CPP_UTILITIES_COMPILER_IS_ADSP 1

#elif defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ICC)
# undef CPP_UTILITIES_COMPILER_IS_IAR
# define CPP_UTILITIES_COMPILER_IS_IAR 1


#endif

#  if CPP_UTILITIES_COMPILER_IS_GNU

#    if !((__GNUC__ * 100 + __GNUC_MINOR__) >= 404)
#      error Unsupported compiler version
#    endif

# if defined(__GNUC__)
#  define CPP_UTILITIES_COMPILER_VERSION_MAJOR (__GNUC__)
# else
#  define CPP_UTILITIES_COMPILER_VERSION_MAJOR (__GNUG__)
# endif
# if defined(__GNUC_MINOR__)
#  define CPP_UTILITIES_COMPILER_VERSION_MINOR (__GNUC_MINOR__)
# endif
# if defined(__GNUC_PATCHLEVEL__)
#  define CPP_UTILITIES_COMPILER_VERSION_PATCH (__GNUC_PATCHLEVEL__)
# endif

#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 408 && __cplusplus >= 201103L
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 1
#    else
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 0
#    endif

#  elif CPP_UTILITIES_COMPILER_IS_Clang

#    if !(((__clang_major__ * 100) + __clang_minor__) >= 301)
#      error Unsupported compiler version
#    endif

# define CPP_UTILITIES_COMPILER_VERSION_MAJOR (__clang_major__)
# define CPP_UTILITIES_COMPILER_VERSION_MINOR (__clang_minor__)
# define CPP_UTILITIES_COMPILER_VERSION_PATCH (__clang_patchlevel__)
# if defined(_MSC_VER)
   /* _MSC_VER = VVRR */
#  define CPP_UTILITIES_SIMULATE_VERSION_MAJOR (_MSC_VER / 100)
#  define CPP_UTILITIES_SIMULATE_VERSION_MINOR (_MSC_VER % 100)
# endif

#    if ((__clang_major__ * 100) + __clang_minor__) >= 301 && __has_feature(cxx_thread_local)
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 1
#    else
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 0
#    endif

#  elif CPP_UTILITIES_COMPILER_IS_AppleClang

#    if !(((__clang_major__ * 100) + __clang_minor__) >= 400)
#      error Unsupported compiler version
#    endif

# define CPP_UTILITIES_COMPILER_VERSION_MAJOR (__clang_major__)
# define CPP_UTILITIES_COMPILER_VERSION_MINOR (__clang_minor__)
# define CPP_UTILITIES_COMPILER_VERSION_PATCH (__clang_patchlevel__)
# if defined(_MSC_VER)
   /* _MSC_VER = VVRR */
#  define CPP_UTILITIES_SIMULATE_VERSION_MAJOR (_MSC_VER / 100)
#  define CPP_UTILITIES_SIMULATE_VERSION_MINOR (_MSC_VER % 100)
# endif
# define CPP_UTILITIES_COMPILER_VERSION_TWEAK (__apple_build_version__)

#    if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_thread_local)
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 1
#    else
#      define CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL 0
#    endif

#  else
#    error Unsupported compiler
#  endif

#  if defined(CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL) && CPP_UTILITIES_COMPILER_CXX_THREAD_LOCAL
#    define CPP_UTILITIES_THREAD_LOCAL thread_local
#  elif CPP_UTILITIES_COMPILER_IS_GNU || CPP_UTILITIES_COMPILER_IS_Clang || CPP_UTILITIES_COMPILER_IS_AppleClang
#    define CPP_UTILITIES_THREAD_LOCAL __thread
#  elif CPP_UTILITIES_COMPILER_IS_MSVC
#    define CPP_UTILITIES_THREAD_LOCAL __declspec(thread)
#  else
// CPP_UTILITIES_THREAD_LOCAL not defined for this configuration.
#  endif

#endif

#endif
