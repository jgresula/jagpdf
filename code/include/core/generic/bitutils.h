// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef BITUTILS_JG2024_H__
#define BITUTILS_JG2024_H__

#include <boost/static_assert.hpp>
#include <interfaces/stdtypes.h>

namespace jag {
namespace detail
{

  template<class T>
  struct bit_traits {};

  template<>
  struct bit_traits<UInt64> {
      typedef UInt type;
  };

  template<>
  struct bit_traits<Int64> {
      typedef Int type;
  };
} // namespace detail


template<class From>
typename detail::bit_traits<From>::type high32bits(From val)
{
    BOOST_STATIC_ASSERT(sizeof(From) == 8);
    typedef detail::bit_traits<From>::type To;
    return static_cast<To>(val >> 32);
}

template<class From>
typename detail::bit_traits<From>::type low32bits(From val)
{
    BOOST_STATIC_ASSERT(sizeof(From) == 8);
    typedef detail::bit_traits<From>::type To;
    return static_cast<To>(val);
}


} // namespace jag

#endif // BITUTILS_JG2024_H__
/** EOF @file */
