// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef ERRLIB_JG1154_H__
#define ERRLIB_JG1154_H__

#include <core/errlib/except.h>
#include <core/generic/assert.h>
#include <msg_errlib.h>


// there are several options how to handle an internal error
//
//  * do nothing - which is not good
//  * throw an exception with a special error code
//    - not ideal as destructors of automatic objects are still invoked during
//      the stack unwinding
//    - clients might ignore this error and continue using the library
# define JAG_INTERNAL_ERROR throw exception_internal_error() << JAGLOC
# define JAG_INTERNAL_ERROR_EXPR(e) if(!(e)){ throw exception_internal_error() << JAGLOC; }
# define JAG_INTERNAL_ERROR_MSG(m) throw exception_internal_error(msg_internal_error_s((m))) << JAGLOC
# define JAG_INTERNAL_ERROR_MSG_CODE(m, c) throw exception_internal_error(msg_internal_error_si((m), (c))) << JAGLOC
//  * abort() immediatelly
//    - the library has no ambitions to recover from an internal error
//    - too radical for a library

#ifdef JAG_DEBUG
# define JAG_ONLY_IN_DEBUG(expr) expr

// # define JAG_INTERNAL_ERROR JAG_ASSERT(!"internal error")
// # define JAG_INTERNAL_ERROR_MSG(m) JAG_ASSERT(!"internal error" && (m))
// # define JAG_INTERNAL_ERROR_MSG_CODE(m, c) JAG_ASSERT(!"internal error" && (m) && (c))
#else
# define JAG_ONLY_IN_DEBUG(expr)
# define JAG_INTERNAL_ERROR_ABORT {                                              \
    fprintf(stderr, "Jagpdf encountered an unexpected situation. Aborting.");  \
    abort(); }
#endif


#endif // ERRLIB_JG1154_H__
/** EOF @file */
