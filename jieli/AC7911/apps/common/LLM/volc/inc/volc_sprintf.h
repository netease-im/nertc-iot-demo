#ifndef VOLC_SPRINTF_H_INCLUDED
#define VOLC_SPRINTF_H_INCLUDED

#include "system/includes.h"

typedef struct {
    const char *fmt;
    va_list *va;
} volc_vaformat_t;

#define VOLC_UNUSED(x) ((void)x)

typedef int64_t intmax_t;

#define PRINTF_DISABLE_SUPPORT_FLOAT   0
// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_FTOA_BUFFER_SIZE    32U
#endif

// support for the floating point type (%f)
// default: activated
#if !PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_SUPPORT_FLOAT
#endif

// support for exponential floating point notation (%e/%g)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_SUPPORT_EXPONENTIAL
#endif

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION 6U
#endif

// define the largest float suitable to print with %f
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#define PRINTF_MAX_FLOAT 1e9
#endif

// support for the long long types (%llu or %p)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_SUPPORT_LONG_LONG
#endif

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#define PRINTF_SUPPORT_PTRDIFF_T
#endif

///////////////////////////////////////////////////////////////////////////////

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

#define PRINTF_SUPPORT_FLOAT

// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#include <float.h>
#endif

typedef unsigned long size_t ;

// output function type
typedef void (*out_fct_type)(char character, void *buffer, size_t idx, size_t maxlen);

// wrapper (used as buffer) for output function type
typedef struct {
    void (*fct)(char character, void *arg);
    void *arg;
} out_fct_wrap_type;

int volc_vsnprintf(char *buffer, size_t count, const char *format, va_list va);
int volc_snprintf(char *buffer, size_t count, const char *format, ...);
#endif // VOLC_SPRINTF_H_INCLUDED

