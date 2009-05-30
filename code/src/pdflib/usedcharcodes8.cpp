// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "usedcharcodes8.h"
#include <core/generic/assert.h>
#include <core/generic/minmax.h>

namespace jag { namespace pdf
{

//////////////////////////////////////////////////////////////////////////
UsedCharCodes8::UsedCharCodes8()
    : m_bounds(255, 0)
{
}

//////////////////////////////////////////////////////////////////////////
void UsedCharCodes8::on_char_codes(Char const* seq, size_t byte_length)
{
    JAG_PRECONDITION(byte_length >= 0);

    unsigned char const* it = static_cast<unsigned char const*>(static_cast<void const*>(seq));
    unsigned char const* end = it + byte_length;
    while(it < end)
    {
        m_bounds.first = (min)(m_bounds.first, static_cast<UChar32>(*it));
        m_bounds.second = (max)(m_bounds.second, static_cast<UChar32>(*it));
        ++it;
    }
}

//////////////////////////////////////////////////////////////////////////
bool UsedCharCodes8::is_empty() const
{
    return m_bounds.first > m_bounds.second;
}

//////////////////////////////////////////////////////////////////////////
UsedCharCodes8::Range UsedCharCodes8::whole_range() const
{
    JAG_ASSERT(!is_empty());
    return m_bounds;
}


}} //namespace jag::pdf
