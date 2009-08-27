// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <cstdio>
#include <stdarg.h>
#include <locale.h>

#include <core/jstd/crt.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/minmax.h>

namespace jag {
namespace jstd {

//////////////////////////////////////////////////////////////////////////
int snprintf(Char* buffer, int count, Char const* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    int ret_val = static_cast<int>(JAG_VSNPRINTF(buffer, count, format, arguments));
    va_end(arguments);

    // check if formatting has not exceeded the size of the buffer
    JAG_ASSERT(ret_val < count);
    if (ret_val == count)
    {
        buffer[count-1]=0;
        --ret_val;
    }

    return ret_val;
}

//////////////////////////////////////////////////////////////////////////
int snprintf_no_fail(Char* buffer, int count, Char const* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    int ret_val = static_cast<int>(JAG_VSNPRINTF(buffer, count, format, arguments));
    va_end(arguments);

    if (ret_val<0 || ret_val >= count)
        JAG_INTERNAL_ERROR;

    return ret_val;
}


//
//
//
int snprintf_pdf_double(Char* buffer, int count, double value)
{
    // A seemingly simple task of printing a double with at most 5
    // significant decimal digits of precision in fractional part does not
    // have a straightforward solution:
    //
    // std::ostrstream .. deprecated
    // std::ostringstream .. allocation + slow
    // boost::format .. slow and maybe allocation
    // printf %e .. exponent
    // printf %g .. cannot specify the number of digits after the decimal
    //              point (%. specifies significant digits); moreover in
    //              certain cases can print exponent
    // printf %f .. always prints trailing zeros
    //
    //
    // To get a result consistent across multiple platforms we do the
    // following:
    //  - round the number to .00001
    //  - format it with %f forcing to always print the fractional part (by
    //    using #)
    //  - discard the trailing zeroes and possibly the decimal point
    //  - check the current locale and if the decimal point is not '.' then
    //    replace it (fixes problem with 'import gtk')
    value = (min)(value, 3.403e+38);
    value = (max)(value, -3.403e+38);
    double rounded = round(value, .00001);
    if (rounded == 0.0) rounded = 0.0; // negative zero -> zero
    int written = jstd::snprintf(buffer, count, "%#.5f", rounded);
    
    char* p = buffer + written - 1;
    while(*p == '0') --p;

    lconv * lc;
    lc = localeconv();
    JAG_ASSERT(lc->decimal_point);
    const char decimal_point = lc->decimal_point[0];
    
    if (*p != decimal_point)
    {
        if (decimal_point != '.')
        {
            char* c = p;
            while (*c != decimal_point && c > buffer) --c;
            JAG_ASSERT(c > buffer); // decimal point should be always present
            *c = '.';
        }
        ++p;
    }

    // p points one character past the last digit
    *p = 0;
    return p - buffer;
}


}} //namespace jag:jstd
