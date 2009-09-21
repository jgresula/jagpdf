// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef THREAD_UTILS_JG236_H__
#define THREAD_UTILS_JG236_H__

#include <interfaces/stdtypes.h>

namespace jag {
namespace jstd {

//
//
//
inline ULong current_thread_id()
{
    return ::GetCurrentThreadId();
}


}} // namespace jag::jstd

#endif // THREAD_UTILS_JG236_H__
/** EOF @file */
