// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __HANDLE_H_JAG_2043__
#define __HANDLE_H_JAG_2043__

#include <interfaces/stdtypes.h>
#include <core/generic/assert.h>


namespace jag {

/**
 * @defgroup group_THandle THandle interface
 * @{
 */

/**
 * @brief Type safe handle representation.
 * @param T an arbitrary type
 *
 * An invalid value is 0.
 */
template<class T>
class THandle
{
public:
    enum { INVALID_VALUE = 0 };

    explicit THandle(UInt id)
        : m_id(id)
    {
        JAG_PRECONDITION_MSG(id, "non-zero value expected");
    }

    THandle()
        : m_id(INVALID_VALUE)
    {}



    /// retrieves integral handle value
    UInt id() const { return m_id; }

    /// resets handle value to the passed value
    void reset(THandle const& other) { m_id = other.m_id; }

public: // operations
    THandle& bitwise_or(UInt val) { m_id |= val; return *this; }
    THandle& bitwise_and(UInt val) { m_id &= val; return *this; }
    THandle& minus(UInt val) { m_id -= val; return *this; }
    THandle& plus(UInt val) { m_id += val; return *this; }
    THandle& lshift(UInt val) { m_id <<= val; return *this; }
    THandle& rshift(UInt val) { m_id >>= val; return *this; }


private:
    UInt    m_id;
};



/// finds out whether the given handle is valid
template<class T>
bool is_valid(THandle<T> const& lhs)
{
    return lhs.id() != THandle<T>::INVALID_VALUE;
}


//////////////////////////////////////////////////////////////////////////
template<class T>
bool operator<(THandle<T> const& lhs,  THandle<T> const& rhs)
{
    return lhs.id() < rhs.id();
}

//////////////////////////////////////////////////////////////////////////
template<class T>
bool operator>(THandle<T> const& lhs,  THandle<T> const& rhs)
{
    return lhs.id() > rhs.id();
}


//////////////////////////////////////////////////////////////////////////
template<class T>
bool operator==(THandle<T> const& lhs,  THandle<T> const& rhs)
{
    return lhs.id() == rhs.id();
}

//////////////////////////////////////////////////////////////////////////
template<class T>
bool operator!=(THandle<T> const& lhs,  THandle<T> const& rhs)
{
    return !(lhs==rhs);
}


/// factory function creating a handle from an integral value
template<class T>
THandle<T> handle_from_id(UInt id)
{
    return THandle<T>(id);
}

/// retrieves integral value from a handle
template<class ID, class T>
ID id_from_handle(THandle<T> handle)
{
    return handle.id();
}

/**
 * @brief transforms handle to an index
 *
 * Indices are zero based, e.g. handle with value 1 is mapped to 0, 2->1, etc.
 */
template<class T>
UInt index_from_handle(THandle<T> handle)
{
    JAG_PRECONDITION(is_valid(handle));
    return handle.id() - 1;
}


/** @} */

} //namespace jag


#endif //__HANDLE_H_JAG_2043__
