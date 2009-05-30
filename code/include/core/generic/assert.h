// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __ASSERT_H_JG1232__
#define __ASSERT_H_JG1232__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <cassert>
#include <stdio.h>
#include <stdlib.h>

#define JAG_ASSERT assert
#define JAG_ASSERT_MSG(expr, msg) assert((expr) && msg)

#define JAG_PRECONDITION(expr) JAG_ASSERT_MSG((expr), "precondition violation")
#define JAG_PRECONDITION_MSG(expr,msg) JAG_ASSERT_MSG((expr), "precondition violation: " msg)
#define JAG_POSTCONDITION(expr) JAG_ASSERT_MSG((expr), "postcondition violation")
#define JAG_INVARIANT(expr) JAG_ASSERT_MSG((expr), "invariant violation")
#define JAG_TBD JAG_ASSERT(!"to be done")
#define JAG_TO_BE_TESTED JAG_ASSERT(!"to be tested")


#endif //__ASSERT_H_JG1232__

