// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __STATICPODARRAY_H_JAG_2200__
#define __STATICPODARRAY_H_JAG_2200__

#include <interfaces/stdtypes.h>
#include <boost/type_traits/is_pod.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <string.h>

namespace jag
{

/// static pod array with copy semantics
template<class T,int SIZE>
class StaticPODArray
{
    BOOST_STATIC_ASSERT(boost::is_pod<T>::value);

public:
    enum { MAX_ITEMS = SIZE };
    StaticPODArray()
        : m_size()
    {}

    StaticPODArray(T const* data, size_t size) {
        reset(data, size);
    }

    void reset(StaticPODArray const& other) {
        reset(other.m_data, other.m_size);
    }

    void reset(T const* data, size_t size) {
        m_size = (std::min)(size, static_cast<size_t>(MAX_ITEMS));
        memcpy(m_data, data, size*sizeof(T));
    }

    size_t size() const { return m_size; }
    T const* data() const { return m_data; }

private:
    size_t    m_size;
    T        m_data[MAX_ITEMS];
};



//////////////////////////////////////////////////////////////////////////
template<class T,int SIZE>
inline bool is_valid(StaticPODArray<T,SIZE> const& obj)
{
    return obj.size() ? true : false;
}

} // namespace jag::resources

#endif //__STATICPODARRAY_H_JAG_2200__

