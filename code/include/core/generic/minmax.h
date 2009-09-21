// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MINMAX_H_JG2149__
#define __MINMAX_H_JG2149__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif

#include <cstddef>
#include <interfaces/stdtypes.h>

namespace jag
{

#define JAG_PREVENT_MACRO_SUBSTITUTION
#define JAG_MINMAX_OVERLOAD(TYPE)                                                                \
    inline TYPE min JAG_PREVENT_MACRO_SUBSTITUTION (TYPE a, TYPE b) { return b < a ? b : a; }    \
    inline TYPE max JAG_PREVENT_MACRO_SUBSTITUTION (TYPE a, TYPE b) { return a < b ? b : a; }

template <class T>
inline T const & min JAG_PREVENT_MACRO_SUBSTITUTION (T const& a, T const& b)
{
    return b < a ? b : a;
}

template <class T>
inline T const & max JAG_PREVENT_MACRO_SUBSTITUTION (T const& a, T const& b)
{
    return a < b ? b : a;
}

JAG_MINMAX_OVERLOAD(int);
JAG_MINMAX_OVERLOAD(size_t);
JAG_MINMAX_OVERLOAD(short);
JAG_MINMAX_OVERLOAD(unsigned short);
JAG_MINMAX_OVERLOAD(char);
JAG_MINMAX_OVERLOAD(unsigned char);
JAG_MINMAX_OVERLOAD(double);
JAG_MINMAX_OVERLOAD(float);

#ifndef JAG_64BIT_ADDRESS_MODEL
JAG_MINMAX_OVERLOAD(ULong);
#endif 


#undef JAG_PREVENT_MACRO_SUBSTITUTION
#undef JAG_MINMAX_OVERLOAD


} //namespace jag


#endif //__MINMAX_H_JG2149__

