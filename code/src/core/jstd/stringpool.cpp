// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/stringpool.h>
#include <string.h>

namespace jag {
namespace jstd {

StringPool::StringPool(size_t size)
    : m_pool(sizeof(Char), size)
{
}

Char const* StringPool::add(Char const* str)
{
    const size_t len = strlen(str) + 1;
    Char* copy = static_cast<Char*>(m_pool.ordered_malloc(len));
    memcpy(copy, str, len);
    return copy;
}

Char const* StringPool::add(Char const* begin, Char const* end)
{
    const ptrdiff_t len = end-begin;
    Char* copy = static_cast<Char*>(m_pool.ordered_malloc(len+1));
    memcpy(copy, begin, len);
    copy[len]=0;
    return copy;
}



}} // namespace jag::jstd

/** EOF @file */
