/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#ifndef TESTCOMMON_JG202_H__
#define TESTCOMMON_JG202_H__

#include <stdio.h>
#include <stdlib.h>
#include <jagpdf/api.h>


#define JAGC_TEST(expr) if (!(expr)) {                              \
        printf(__FILE__ "(%d) test '" #expr "' failed.\n", __LINE__); \
        abort();                                                        \
    }

#define JAGC_FAIL                                                      \
        printf(__FILE__ "(%d) unconditionally failed.\n", __LINE__); \
        abort()


jag_Int JAG_CALLSPEC write_noop(void* custom, void const* data, jag_ULong size);
jag_Int JAG_CALLSPEC close_noop(void* custom);
jag_streamout get_noop_stream();

#define JAG_IGNORE_UNUSED(v) (void)(v)

#endif
/** EOF @file */
