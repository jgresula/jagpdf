// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __AUTOARRAY_JG2155_H__
# define __AUTOARRAY_JG2155_H__

#include <core/generic/assert.h>

namespace jag {

template<class T>
class auto_array
{
    T* m_p;

public:
    auto_array(size_t size)
        : m_p(new T[size])
    {
        JAG_PRECONDITION(size);
    }

    ~auto_array() {
        delete [] m_p;
    }

    T* ptr() {
        JAG_ASSERT(m_p);
        return m_p;
    }

    T* detach() {
        T* tmp = m_p;
        m_p = 0;
        return tmp;
    }
};

} // namespace jag

#endif // __AUTOARRAY_JG2155_H__
/** EOF @file */
