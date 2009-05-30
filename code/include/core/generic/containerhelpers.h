// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CONTAINERHELPERS_H_JG_2251__
#define __CONTAINERHELPERS_H_JG_2251__

#include <vector>

namespace jag
{


/// retrieves byte size of the vector content
template<class T>
size_t byte_size(std::vector<T> const& vec)
{
    return vec.size() * sizeof(T);
}


/// retrieves a pointer to first item of the vector
template<class T>
T const* address_of(std::vector<T> const& vec)
{
    return vec.empty()
        ? static_cast<T*>(0)
        : &vec[0]
    ;
}


} // namespace jag

#endif //__CONTAINERHELPERS_H_JG_2251__

