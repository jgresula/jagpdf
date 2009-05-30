// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MATH_H_JG_0746__
#define __MATH_H_JG_0746__

#include <core/generic/assert.h>

namespace jag  {

//3.14159 26535 89793 23846 26433 83279 50288 41971 69399 37510
const Double PI = 3.14159265358979;//323846264338327950288419716939937510;

/**
 * @brief calculates integer part of log2
 * @param val value to calculate log2 from
 * @pre val>0
 * @return integer part of log2(val)
 */
inline unsigned int slow_log2(unsigned int val)
{
    JAG_PRECONDITION_MSG(val, "non zero value expected");

    unsigned int msb = 0;
    while (val)
    {
        val >>= 1;
        ++msb;
    }

    return msb-1;
}


} // namespace jag


#endif //__MATH_H_JG_0746__

