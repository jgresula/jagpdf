// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <string.h>

#ifndef __CRT_GCC_H__1222252
#define __CRT_GCC_H__1222252

#define JAG_VSNPRINTF vsnprintf
#define JAG_STRICMP strcasecmp

namespace jag {
namespace jstd {

inline char* strerror(int errnum, char* buff, size_t buff_size)
{
    return strerror_r(errnum, buff, buff_size);
}

}} // namespace jag::jstd


#endif //__CRT_GCC_H__1222252
