// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef STRINGPOOL_JG954_H__
#define STRINGPOOL_JG954_H__

#include <interfaces/stdtypes.h>
#include <boost/pool/poolfwd.hpp>
#include <boost/pool/pool.hpp>

namespace jag {
namespace jstd {

class StringPool
{
    boost::pool<> m_pool;

public:
    StringPool(size_t size);
    Char const* add(Char const* str);
    Char const* add(Char const* begin, Char const* end);
};



}} // namespace jag::jstd

#endif // STRINGPOOL_JG954_H__
/** EOF @file */
