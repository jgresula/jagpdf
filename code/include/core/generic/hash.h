// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __HASH_H_JAG_1139__
#define __HASH_H_JAG_1139__


namespace jag
{


template<class T>
struct hash_functor
    : public std::unary_function<T const&, size_t>
{

    size_t operator()(T const& t) {
        return hash_value(t);
    }
};


} // namespace jag::resources

#endif //__HASH_H_JAG_1139__

