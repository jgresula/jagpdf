// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/arcfour_stream.h>
#include <core/generic/assert.h>
#include <algorithm>

namespace jag { namespace jstd
{

ArcFourStream::ArcFourStream(ISeqStreamOutput& out_stream, Byte const* key, int bit_length)
    : m_i(0)
    , m_j(0)
    , m_out_stream(out_stream)
{
    JAG_ASSERT_MSG(!(bit_length % 8), "bad encryption key length");

    int key_byte_length = bit_length / 8;
    for(int i=0; i<256; ++i)
    {
        m_s[i] = static_cast<Byte>(i);
    }

    int j=0;
    for (int i=0; i<256; ++i)
    {
        j = (j + m_s[i] + key[i%key_byte_length]) % 256;
        std::swap(m_s[i], m_s[j]);
    }
}

void ArcFourStream::write(void const* data, ULong size)
{
    const int buffer_size = 16384;
    Byte buffer[buffer_size];
    Byte const* end = buffer + buffer_size;
    Byte* it = buffer;


    Byte const* data_to_encrypt = static_cast<Byte const*>(data);
    for (UInt k=0; k<size; ++k)
    {
        m_i = ++m_i % 256;
        m_j = (m_j + m_s[m_i]) % 256;
        std::swap(m_s[m_i], m_s[m_j]);
        if (it == end)
        {
            m_out_stream.write(buffer, buffer_size);
            it = buffer;
        }
        *it++ = data_to_encrypt[k] ^ m_s[(m_s[m_i]+m_s[m_j]) % 256 ];
    }
    m_out_stream.write(buffer, static_cast<UInt>(it-buffer));
}

ULong ArcFourStream::tell() const
{
    return m_out_stream.tell();
}

void ArcFourStream::flush()
{
    return m_out_stream.flush();
}

}} // namespace jag::jstd
