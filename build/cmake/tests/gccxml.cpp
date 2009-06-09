// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

// gccxml sanity test

#if !defined(_WIN32)
// causes problem on vanilla ubuntu 9.04
# include <unistd.h>
#endif

#include <vector>

enum E { V } __attribute__((gccxml("test")));

void fun() __attribute__((gccxml("test")));

class __attribute__((gccxml("test"))) Cls {
    virtual void fun() __attribute__((gccxml("test"))) = 0;
};

/** EOF @file */
