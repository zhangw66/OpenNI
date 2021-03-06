#ifndef __TYPT_H___
#define __TYPT_H___
#ifdef WIN32
#  ifndef _WIN32
#    define _WIN32
#  endif
#endif

#ifdef _WIN32
# ifndef _STDINT_H
#  if defined(_MSC_VER) && _MSC_VER < 1600
    typedef __int8            int8_t;
    typedef __int16           int16_t;
    typedef __int32           int32_t;
    typedef __int64           int64_t;
    typedef unsigned __int8   uint8_t;
    typedef unsigned __int16  uint16_t;
    typedef unsigned __int32  uint32_t;
    typedef unsigned __int64  uint64_t;
#  else
#   include <stdint.h>
#  endif
# endif
#else
# include <stdint.h>
#endif
//#define ADI_TOF_SDK
#define TY_SDK
//#define SUNYU_TOF_SKD
#endif
