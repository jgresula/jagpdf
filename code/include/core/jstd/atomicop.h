// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef ATOMICOP_JG1535_H__
#define ATOMICOP_JG1535_H__

#include <boost/config.hpp>

#ifdef _WIN32
# include <windows.h>

namespace jag {
namespace jstd {


long atomic_increment(long volatile* var) {
    return ::InterlockedIncrement(var);
}

long atomic_decrement(long volatile* var) {
   return ::InterlockedDecrement(var);
}

}} // namespace jag::jstd

#else

namespace jag {
namespace jstd {

// TBD

long atomic_increment(long volatile* var) {
    return ++*var;
}

long atomic_decrement(long volatile* var) {
    return --*var;
}

}} // namespace jag::jstd

#endif



#endif // ATOMICOP_JG1535_H__
/** EOF @file */
