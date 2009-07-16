// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FLOATPOINTTOOLS_H_JG033__
#define __FLOATPOINTTOOLS_H_JG033__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <core/generic/assert.h>
#include <interfaces/stdtypes.h>
#include <limits>
#include <cmath>

namespace jag
{

namespace
{
    const Double double_cmp_delta = 1e-5;
} // anonymous namespace


/**
 * @brief compares two doubles for equality
 *
 * for details see: C++ FAQ [29.17] (http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.17)
 *
 * @param lhs left hand operand
 * @param rhs right hand operand
 *
 * @return true of the values are 'equal', false otherwise
 */
inline bool equal_doubles(Double lhs, Double rhs)
{
    return fabs(rhs-lhs) <= double_cmp_delta * fabs(rhs);
}

/// compares given double with zero
inline bool equal_to_zero(double rhs)
{
    return fabs(rhs) <= 1e-8;
}



//////////////////////////////////////////////////////////////////////////
inline void set_invalid_double(Double &var)
{
    var = std::numeric_limits<Double>::infinity();
}

//////////////////////////////////////////////////////////////////////////
inline bool is_invalid_double(Double const& var)
{
    JAG_PRECONDITION(
        std::numeric_limits<Double>::infinity() == std::numeric_limits<Double>::infinity()
   );

    return var == std::numeric_limits<Double>::infinity();
}

/// rounds given value
inline int round(double val)
{
    return static_cast<int>(val>0 ? val+0.5 : val-0.5);
}

//
//
//
inline double round(double val, double precision)
{
    // avoiding excessive precision - volatile disables optimization for d2
    volatile double d2 = val / precision;
    return precision * (val > 0 ? floor(d2 + .5) : ceil(d2 - .5));
}



} //namespace jag


#endif //__FLOATPOINTTOOLS_H_JG033__

