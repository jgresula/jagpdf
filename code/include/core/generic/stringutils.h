// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __STRINGUTILS_H_JG2332__
#define __STRINGUTILS_H_JG2332__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <cstring>
#include <ctype.h>
#include <functional>

#if defined(JAG_INCLUDE_STRING_UTILS_EX)
# include <boost/algorithm/string/trim.hpp>
# include <boost/range/iterator_range.hpp>
#endif

namespace jag {
//////////////////////////////////////////////////////////////////////////
struct string_compare
{
    bool operator()(char const* lhs, char const* rhs) const
    {
        return strcmp(lhs, rhs) < 0;
    }
};



//////////////////////////////////////////////////////////////////////////
struct string_less
{
    bool operator()(char const* lhs, char const* rhs) const
    {
        return strcmp(lhs, rhs) < 0;
    }
};



//////////////////////////////////////////////////////////////////////////
inline bool is_empty(Char const* val)
{
    return !val || !val[0];
}


inline Char const* safe_null_string(Char const* str)
{
    return str ? str : "";
}



/// standard isspace as an adaptable funtion object
struct fn_isspace
    : public std::unary_function<Char,bool>
{
    bool operator()(Char c) const
    {
        return isspace(c);
    }
};


// ---------------------------------------------------------------------------
//                      string utils-ex


#if defined(JAG_INCLUDE_STRING_UTILS_EX)

//
//
//
inline boost::iterator_range<char const*>
trim_range(char const* begin, char const* end)
{
    return boost::iterator_range<char const*>(
        boost::algorithm::detail::trim_begin(begin, end, boost::algorithm::is_space()),
        boost::algorithm::detail::trim_end(begin, end, boost::algorithm::is_space()));
}


#endif // JAG_INCLUDE_STRING_UTILS_EX



} //namespace jag


#endif //__STRINGUTILS_H_JG2332__


