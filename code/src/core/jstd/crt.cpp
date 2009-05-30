// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <cstdio>
#include <stdarg.h>

#include <core/jstd/crt.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>

namespace jag { namespace jstd
{

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


}} //namespace jag:jstd
