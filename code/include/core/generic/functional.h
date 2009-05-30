// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FUNCTIONAL_JG2232_H__
# define __FUNCTIONAL_JG2232_H__

#include <functional>

namespace jag {


template <class Pair>
struct jag_select2nd
    : public std::unary_function<Pair, typename Pair::second_type>
{
    const typename Pair::second_type operator()(Pair const& val) const
    {
        return val.second;
    }
};


} // namespace jag

#endif // __FUNCTIONAL_JG2232_H__
/** EOF @file */
