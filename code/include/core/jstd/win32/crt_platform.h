// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CRT_MSVC_H__1222244
#define __CRT_MSVC_H__1222244

#include <windows.h>

#define JAG_VSNPRINTF _vsnprintf
#define JAG_STRICMP _stricmp

namespace jag {
namespace jstd {

inline char* strerror(int errnum, char* buff, size_t buff_size)
{
    if (strerror_s(buff, buff_size, errnum))
    {
        strncpy(buff, "Unknown Error", buff_size);
    }
    return buff;
}

}} //namespace jag::jstd

#endif //__CRT_MSVC_H__1222244
