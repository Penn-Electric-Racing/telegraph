#ifndef __STDINT_H__
#define __STDINT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if __SCHAR_MAX__ == 0x7f
typedef signed char int8_t ;
typedef unsigned char uint8_t ;
#define ___int8_t_defined 1
#endif

#if __INT_MAX__ == 0x7fff
typedef signed int int16_t;
typedef unsigned int uint16_t;
#define ___int16_t_defined 1
#elif __SHRT_MAX__ == 0x7fff
typedef signed short int16_t;
typedef unsigned short uint16_t;
#define ___int16_t_defined 1
#elif __SCHAR_MAX__ == 0x7fff
typedef signed char int16_t;
typedef unsigned char uint16_t;
#define ___int16_t_defined 1
#endif

#if ___int16_t_defined
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
#define ___int_least16_t_defined 1
#if !___int8_t_defined
typedef int16_t int_least8_t;
typedef uint16_t uint_least8_t;
#define ___int_least8_t_defined 1
#endif
#endif

#if __INT_MAX__ == 0x7fffffffL
typedef signed int int32_t;
typedef unsigned int uint32_t;
#define ___int32_t_defined 1
#elif __LONG_MAX__ == 0x7fffffffL
typedef signed long int32_t;
typedef unsigned long uint32_t;
#define ___int32_t_defined 1
#elif __SHRT_MAX__ == 0x7fffffffL
typedef signed short int32_t;
typedef unsigned short uint32_t;
#define ___int32_t_defined 1
#elif __SCHAR_MAX__ == 0x7fffffffL
typedef signed char int32_t;
typedef unsigned char uint32_t;
#define ___int32_t_defined 1
#endif

#if ___int32_t_defined
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
#define ___int_least32_t_defined 1

#if !___int8_t_defined
typedef int32_t int_least8_t;
typedef uint32_t uint_least8_t;
#define ___int_least8_t_defined 1
#endif
#if !___int16_t_defined
typedef int32_t int_least16_t;
typedef uint32_t uint_least16_t;
#define ___int_least16_t_defined 1
#endif
#endif
#if __LONG_MAX__ > 0x7fffffff
typedef signed long int64_t;
typedef unsigned long uint64_t;
#define ___int64_t_defined 1

/* GCC has __LONG_LONG_MAX__ */
#elif  defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#define ___int64_t_defined 1
/* POSIX mandates LLONG_MAX in <limits.h> */
#elif  defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#define ___int64_t_defined 1
#elif  __INT_MAX__ > 0x7fffffff
typedef signed int int64_t;
typedef unsigned int uint64_t;
#define ___int64_t_defined 1
#endif

#ifdef __cplusplus
}
#endif

#endif
