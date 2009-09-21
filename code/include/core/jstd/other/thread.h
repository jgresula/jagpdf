// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef THREAD_UTILS_JG1649_H__
#define THREAD_UTILS_JG1649_H__

#include <pthread.h>
#include <boost/static_assert.hpp>
#include <interfaces/stdtypes.h>

namespace jag {
namespace jstd {


inline ULong current_thread_id()
{
    BOOST_STATIC_ASSERT(sizeof(ULong)==sizeof(pthread_t));
    return (unsigned)pthread_self();
//    return reinterpret_cast<unsigned>(static_cast<void*>());
}


}} // namespace jag::jstd

#endif // THREAD_UTILS_JG1649_H__
/** EOF @file */
