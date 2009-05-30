// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CRT_H_GENERIC_822006
#define __CRT_H_GENERIC_822006

#include <core/jstd/crt_platform.h>
#include <interfaces/stdtypes.h>


namespace jag {
/// portable crt based functionality
namespace jstd {
    /**
    *  safe sprintf formatting, produces zero-terminated string
    *
    *  @param buffer destination
    *  @param count buffer size
    *  @param format formatting string
    *
    *  @return number of characters written (null is not included)
    */
    int snprintf(Char* buffer, int count, Char const* format, ...);


    /**
    *  Safe sprintf formatting, produces zero-terminated string.
    *
    *  @param buffer destination
    *  @param count buffer size
    *  @param format formatting string
    *
    *  Insufficient buffer length results to an internal error
    *
    *  @return number of characters written (null is not included)
    */
    int snprintf_no_fail(Char* buffer, int count, Char const* format, ...);

}} //namespace jag:jstd


#endif //__CRT_H__822006
