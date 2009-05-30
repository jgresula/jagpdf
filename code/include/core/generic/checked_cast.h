// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CHECKED_CAST_H__
#define __CHECKED_CAST_H__

#include <core/generic/assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

namespace jag
{

/**
 * @brief in debug mode performs checked static class, in release it is classic static_cast
 *
 * @param To target type
 * @param From source type
 *
 * @param from pointer to the source object
 */
template<class To, class From> To checked_static_cast(From* from)
{
    JAG_ASSERT_MSG(dynamic_cast<To>(from) == static_cast<To>(from), "checked_cast failed");
    return static_cast<To>(from);
}

/**
 * @brief in debug mode performs checked static class, in release it is classic static_cast
 *
 * @param To target type
 * @param From source type
 *
 * @param from reference to the source object
 */
template<class To, class From> To checked_static_cast(From& from)
{
    JAG_ASSERT_MSG(dynamic_cast<From*>(&from) == static_cast<To*>(&from), "checked_cast failed");
    return static_cast<To>(from);
}

/**
* @brief in debug mode performs checked static class on shared_ptr, in release it is classic static_cast
*
* @param To target type
* @param From source type
*
* @param from reference to the source object
*/
template<class To, class From> boost::shared_ptr<To> checked_static_pointer_cast(boost::shared_ptr<From> const& from)
{
    JAG_ASSERT_MSG(boost::dynamic_pointer_cast<To>(from) == boost::static_pointer_cast<To>(from), "checked_cast failed");
    return boost::static_pointer_cast<To>(from);
}

//////////////////////////////////////////////////////////////////////////
template<class To, class From>
boost::intrusive_ptr<To> checked_static_pointer_cast(boost::intrusive_ptr<From> const& from)
{
    JAG_ASSERT_MSG(boost::dynamic_pointer_cast<To>(from) == boost::static_pointer_cast<To>(from), "checked_cast failed");
    return boost::static_pointer_cast<To>(from);
}


/// 'safe' reinterpret cast (see C++ coding standards, item 92)
template<class To, class From>
To jag_reinterpret_cast(From* p)
{
    return static_cast<To>(static_cast<void*>(p));
}

/// 'safe' reinterpret cast, const version (see C++ coding standards, item 92)
template<class To, class From>
To jag_reinterpret_cast(From const* p)
{
    return static_cast<To>(static_cast<void const*>(p));
}



} //namespace jag

#endif //__CHECKED_CAST_H__
